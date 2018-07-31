package free.pstruho.mightylooper

import android.support.v7.app.AppCompatActivity
import android.support.v4.app.Fragment
import android.support.v4.app.FragmentManager
import android.support.v4.app.FragmentPagerAdapter
import android.view.Menu
import android.view.MenuItem
import kotlinx.android.synthetic.main.activity_main.*
import android.content.Intent
import free.pstruho.mightylooper.service.LooperService
import android.os.*
import android.support.v7.app.AlertDialog
import free.pstruho.mightylooper.controls.ControlsFragment
import free.pstruho.mightylooper.mixer.MixerFragment
import free.pstruho.mightylooper.settings.SettingsFragment
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
                1 -> MixerFragment()
                2 -> ControlsFragment()
                else -> throw IllegalArgumentException("Wrong page number. No corresponding fragment exists.")
            }

        }

        override fun getCount(): Int {
            // Show 3 total pages.
            return 3
        }
    }
}
