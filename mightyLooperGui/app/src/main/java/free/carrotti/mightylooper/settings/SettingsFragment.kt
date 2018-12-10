package free.carrotti.mightylooper.settings

import android.content.*
import android.os.Bundle
import android.os.IBinder
import android.support.v4.content.LocalBroadcastManager
import android.support.v7.preference.Preference
import android.support.v7.preference.PreferenceDialogFragmentCompat
import android.support.v7.preference.PreferenceFragmentCompat
import free.carrotti.mightylooper.R
import free.carrotti.mightylooper.service.LooperService
import free.carrotti.mightylooper.utils.*

class SettingsFragment : PreferenceFragmentCompat(), SharedPreferences.OnSharedPreferenceChangeListener {

    private val tempoPreferenceKey by lazy { getString(R.string.tempo_preference_key) }
    private val shouldSendValueToCore by lazy { hashMapOf(
            tempoPreferenceKey to true
    )}

    private lateinit var mLooperService: LooperService
    private var mBound = false

    private val mReceiver = object : BroadcastReceiver() {
        override fun onReceive(context: Context, intent: Intent) {
            val action = intent.action
            when (action) {
                CMD_SET_TEMPO -> {
                    // set 'should send' flag to false because this preference change was invoked by core
                    shouldSendValueToCore[tempoPreferenceKey] = false
                    (findPreference(tempoPreferenceKey) as TempoPreference).update(intent.getIntExtra("${INTENT_ARG_PREFIX}1", 0))
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
        filter.addAction(CMD_SET_TEMPO)
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
            tempoPreferenceKey -> if (shouldSendValueToCore[tempoPreferenceKey] != false) mLooperService.write(buildMessage(CMD_SET_TEMPO, listOf(sharedPreferences?.getInt(key, 0) ?: 0)))
        }
        // if given preference has corresponding 'should send' flag set to false, reset it to true
        shouldSendValueToCore[key]?.let { if(!it) shouldSendValueToCore[key!!] = true }
    }
}