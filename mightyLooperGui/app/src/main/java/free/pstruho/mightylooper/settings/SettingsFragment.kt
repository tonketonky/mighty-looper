package free.pstruho.mightylooper.settings

import android.content.BroadcastReceiver
import android.content.Context
import android.content.Intent
import android.content.IntentFilter
import android.os.Bundle
import android.support.v4.content.LocalBroadcastManager
import android.support.v7.preference.Preference
import android.support.v7.preference.PreferenceDialogFragmentCompat
import android.support.v7.preference.PreferenceFragmentCompat
import free.pstruho.mightylooper.R
import free.pstruho.mightylooper.constants.ACTION_SET_TEMPO

class SettingsFragment : PreferenceFragmentCompat() {

    private val mReceiver = object : BroadcastReceiver() {
        override fun onReceive(context: Context, intent: Intent) {
            val action = intent.action
            when (action) {
                ACTION_SET_TEMPO -> {
                    (findPreference("tempo_preference") as TempoPreference).update(intent.getIntExtra("tempo", 0))
                }
            }
        }
    }

    override fun onCreatePreferences(savedInstanceState: Bundle?, rootKey: String?) {
        setPreferencesFromResource(R.xml.preferences, rootKey)
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        val filter = IntentFilter()
        filter.addAction(ACTION_SET_TEMPO)
        LocalBroadcastManager.getInstance(requireContext()).registerReceiver(mReceiver, filter)
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
}