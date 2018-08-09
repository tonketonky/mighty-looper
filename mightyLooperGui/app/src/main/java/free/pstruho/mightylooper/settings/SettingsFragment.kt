package free.pstruho.mightylooper.settings

import android.content.*
import android.os.Bundle
import android.os.IBinder
import android.support.v4.content.LocalBroadcastManager
import android.support.v7.preference.Preference
import android.support.v7.preference.PreferenceDialogFragmentCompat
import android.support.v7.preference.PreferenceFragmentCompat
import free.pstruho.mightylooper.R
import free.pstruho.mightylooper.service.LooperService
import free.pstruho.mightylooper.utils.ACT_ARG_TEMPO
import free.pstruho.mightylooper.utils.MSG_CMD_SET_TEMPO
import free.pstruho.mightylooper.utils.buildSetTempoMessage

class SettingsFragment : PreferenceFragmentCompat(), SharedPreferences.OnSharedPreferenceChangeListener {

    private lateinit var mLooperService: LooperService
    private var mBound = false

    private val mReceiver = object : BroadcastReceiver() {
        override fun onReceive(context: Context, intent: Intent) {
            val action = intent.action
            when (action) {
                MSG_CMD_SET_TEMPO -> {
                    (findPreference("tempo_preference") as TempoPreference).update(intent.getIntExtra(ACT_ARG_TEMPO, 0))
                }
            }
        }
    }

    private val mConnection = object : ServiceConnection {

        override fun onServiceConnected(className: ComponentName,
                                        service: IBinder) {
            mLooperService = (service as LooperService.LocalBinder).getService()
            mBound = true
        }

        override fun onServiceDisconnected(arg0: ComponentName) {
            mBound = false
        }
    }

    override fun onCreatePreferences(savedInstanceState: Bundle?, rootKey: String?) {
        setPreferencesFromResource(R.xml.preferences, rootKey)
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        // bind looper service
        val intent = Intent(requireContext(), LooperService::class.java)
        requireActivity().bindService(intent, mConnection, Context.BIND_AUTO_CREATE)

        // register broadcast receiver
        val filter = IntentFilter()
        filter.addAction(MSG_CMD_SET_TEMPO)
        LocalBroadcastManager.getInstance(requireContext()).registerReceiver(mReceiver, filter)
    }

    override fun onResume() {
        super.onResume()
        preferenceManager.sharedPreferences.registerOnSharedPreferenceChangeListener(this)
    }

    override fun onPause() {
        super.onPause()
        preferenceManager.sharedPreferences.unregisterOnSharedPreferenceChangeListener(this)
    }

    override fun onDestroy() {
        LocalBroadcastManager.getInstance(requireContext()).unregisterReceiver(mReceiver)
        requireActivity().unbindService(mConnection)
        super.onDestroy()
    }

    override fun onDisplayPreferenceDialog(preference: Preference) {
        when (preference) {
            is TempoPreference, is LooperInUsePreference -> showDialogForPreference(preference)
            else -> super.onDisplayPreferenceDialog(preference)
        }
    }

    private fun showDialogForPreference(preference: Preference) {
        val dialogFragment: PreferenceDialogFragmentCompat? = when (preference) {
            is TempoPreference -> TempoPreference.TempoDialog()
            is LooperInUsePreference -> LooperInUsePreference.LooperInUseDialog()
            else -> null
        }

        val bundle = Bundle(1)
        bundle.putString("key", preference.key)
        dialogFragment?.arguments = bundle
        dialogFragment?.setTargetFragment(this, 0)
        dialogFragment?.show(this.fragmentManager, "android.support.v7.preference.PreferenceFragment.DIALOG")
    }

    override fun onSharedPreferenceChanged(sharedPreferences: SharedPreferences?, key: String?) {
        when(key) {
            "tempo_preference" -> mLooperService.write(buildSetTempoMessage(sharedPreferences?.getInt(key, 0) ?: 0))
        }
    }
}