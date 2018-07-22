package free.pstruho.mightylooper

import android.support.v7.app.AppCompatActivity
import android.support.v7.preference.PreferenceFragmentCompat
import android.support.v7.preference.Preference
import android.support.v4.app.Fragment
import android.support.v4.app.FragmentManager
import android.support.v4.app.FragmentPagerAdapter
import android.support.v7.preference.PreferenceDialogFragmentCompat
import android.view.LayoutInflater
import android.view.Menu
import android.view.MenuItem
import android.view.View
import android.view.ViewGroup
import free.pstruho.mightylooper.settings.LooperInUsePreference
import free.pstruho.mightylooper.settings.TempoPreference

import kotlinx.android.synthetic.main.activity_main.*
import android.content.Intent
import free.pstruho.mightylooper.service.LooperService
import android.os.*
import android.support.v7.app.AlertDialog
import kotlinx.android.synthetic.main.dialog_title.view.*

class MainActivity : AppCompatActivity() {

    /**
     * The [android.support.v4.view.PagerAdapter] that will provide
     * fragments for each of the sections. We use a
     * {@link FragmentPagerAdapter} derivative, which will keep every
     * loaded fragment in memory. If this becomes too memory intensive, it
     * may be best to switch to a
     * [android.support.v4.app.FragmentStatePagerAdapter].
     */
    private var mSectionsPagerAdapter: SectionsPagerAdapter? = null

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        /*window.setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN,
                WindowManager.LayoutParams.FLAG_FULLSCREEN)*/

        // start looper service
        startService(Intent(this, LooperService::class.java))

        // Create the adapter that will return a fragment for each of the three
        // primary sections of the activity.
        mSectionsPagerAdapter = SectionsPagerAdapter(supportFragmentManager)

        // Set up the ViewPager with the sections adapter.
        container.adapter = mSectionsPagerAdapter

    }

    override fun onBackPressed() {
        val builder = AlertDialog.Builder(this)

        val title = layoutInflater.inflate(R.layout.dialog_title, null)
        title.titleText.text = "What to do?"
        builder.setCustomTitle(title)

        // when Quit button pressed stop looper service and finish
        builder.setPositiveButton("Quit") { _, _ ->
            stopService(Intent(this, LooperService::class.java))
            finish()
        }
        // when Suspend button pressed proceed with default back button behaviour
        builder.setNegativeButton("Suspend") { _, _ -> super.onBackPressed()}

        builder.create().show()
    }

    override fun onCreateOptionsMenu(menu: Menu): Boolean {
        // Inflate the menu; this adds items to the action bar if it is present.
        menuInflater.inflate(R.menu.menu_main, menu)
        return true
    }

    override fun onOptionsItemSelected(item: MenuItem): Boolean {
        // Handle action bar item clicks here. The action bar will
        // automatically handle clicks on the Home/Up button, so long
        // as you specify a parent activity in AndroidManifest.xml.
        val id = item.itemId

        if (id == R.id.action_settings) {
            return true
        }

        return super.onOptionsItemSelected(item)
    }

    /**
     * A [FragmentPagerAdapter] that returns a fragment corresponding to
     * one of the sections/tabs/pages.
     */
    inner class SectionsPagerAdapter(fm: FragmentManager) : FragmentPagerAdapter(fm) {

        override fun getItem(position: Int): Fragment {
            // getItem is called to instantiate the fragment for the given page.
            // Return a PlaceholderFragment (defined as a static inner class below).
            return when(position) {
                0 -> SettingsFragment()
                else -> PlaceholderFragment.newInstance(position + 1)
            }

        }

        override fun getCount(): Int {
            // Show 3 total pages.
            return 3
        }
    }

    class SettingsFragment : PreferenceFragmentCompat() {

        override fun onCreatePreferences(savedInstanceState: Bundle?, rootKey: String?) {
            setPreferencesFromResource(R.xml.preferences, rootKey)
        }

        override fun onDisplayPreferenceDialog(preference: Preference) {
            when (preference) {
                is TempoPreference, is LooperInUsePreference -> showDialogForPreference(preference)
                else -> super.onDisplayPreferenceDialog(preference)
            }
        }

        private fun showDialogForPreference(preference: Preference) {
            val dialogFragment: PreferenceDialogFragmentCompat? = when (preference) {
                is TempoPreference -> TempoPreference.TempoPreferenceDialogFragment()
                is LooperInUsePreference -> LooperInUsePreference.SelectLooperDialogFragment()
                else -> null
            }

            val bundle = Bundle(1)
            bundle.putString("key", preference.key)
            dialogFragment?.arguments = bundle
            dialogFragment?.setTargetFragment(this, 0)
            dialogFragment?.show(this.fragmentManager, "android.support.v7.preference.PreferenceFragment.DIALOG")
        }
    }

    /**
     * A placeholder fragment containing a simple view.
     */
    class PlaceholderFragment : Fragment() {

        override fun onCreateView(inflater: LayoutInflater, container: ViewGroup?,
                                  savedInstanceState: Bundle?): View? {

            return inflater.inflate(arguments?.getInt(ARG_FRAGMENT) ?: -1, container, false)
        }

        companion object {
            /**
             * The fragment argument representing the section number for this
             * fragment.
             */
            private const val ARG_FRAGMENT = "fragment"

            /**
             * Returns a new instance of this fragment for the given section
             * number.
             */
            fun newInstance(sectionNumber: Int): PlaceholderFragment {
                val fragment = PlaceholderFragment()
                val args = Bundle()
                args.putInt(ARG_FRAGMENT,
                    when (sectionNumber) {
                        2 -> R.layout.fragment_mixer
                        3 -> R.layout.fragment_controls
                        else -> -1
                    }
                )

                fragment.arguments = args
                return fragment
            }
        }
    }
}
