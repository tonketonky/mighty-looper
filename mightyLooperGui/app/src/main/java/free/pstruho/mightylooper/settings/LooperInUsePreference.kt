package free.pstruho.mightylooper.settings

import android.app.Activity
import android.app.Dialog
import android.bluetooth.BluetoothAdapter
import android.bluetooth.BluetoothDevice
import android.content.*
import android.content.res.TypedArray
import android.support.v4.content.LocalBroadcastManager
import android.support.v7.app.AlertDialog
import android.support.v7.preference.DialogPreference
import android.support.v7.preference.PreferenceDialogFragmentCompat
import android.support.v7.widget.LinearLayoutManager
import android.support.v7.widget.RecyclerView
import android.util.AttributeSet
import android.widget.Button
import android.widget.TextView
import free.pstruho.mightylooper.MainActivity
import free.pstruho.mightylooper.R
import free.pstruho.mightylooper.service.UPDATED_LOOPER_LIST
import java.util.*
import android.os.Bundle
import android.view.*
import kotlinx.android.synthetic.main.dialog_title.view.*

private const val DEFAULT_VALUE = "Disconnected"
private const val REQUEST_ENABLE_BT = 1

class LooperInUsePreference(context: Context, attrs: AttributeSet) : DialogPreference(context, attrs) {

    private lateinit var mSelectedLooper: String

    override fun onGetDefaultValue(a: TypedArray, index: Int): Any {
        return a.getString(index)
    }

    override fun onSetInitialValue(restorePersistedValue: Boolean, defaultValue: Any?) {
        update( if (restorePersistedValue) getPersistedString(DEFAULT_VALUE) else defaultValue as String )
    }

    fun update(looperName: String) {
        persistString(looperName)
        this.mSelectedLooper = looperName
        this.summary = looperName
    }

    class SelectLooperDialogFragment : PreferenceDialogFragmentCompat() {

        private lateinit var mInflater: LayoutInflater
        private lateinit var mSelectLooperDialogView: View
        private lateinit var mLooperListViewAdapter: LooperListViewAdapter

        private val mReceiver = object : BroadcastReceiver() {
            override fun onReceive(context: Context, intent: Intent) {
                val action = intent.action
                when (action) {
                    UPDATED_LOOPER_LIST -> {
                        mLooperListViewAdapter.updateLooperList((activity as MainActivity).mLooperService.getDeviceList())
                        mLooperListViewAdapter.notifyDataSetChanged()
                    }
                }
            }
        }

        override fun onCreateDialog(savedInstanceState: Bundle?): Dialog {
            // get alert builder and inflater
            val builder = AlertDialog.Builder(requireContext())
            mInflater = requireActivity().layoutInflater

            // inflate and set title
            val title = mInflater.inflate(R.layout.dialog_title, null)
            title.titleText.text = "Looper in use"
            builder.setCustomTitle(title)

            // inflate and set view
            mSelectLooperDialogView = mInflater.inflate(R.layout.dialog_select_looper, null)
            builder.setView(mSelectLooperDialogView)

            // register broadcast receiver
            val filter = IntentFilter(UPDATED_LOOPER_LIST)
            LocalBroadcastManager.getInstance(requireContext()).registerReceiver(mReceiver, filter)

            // set up looper list view
            val looperListView= mSelectLooperDialogView.findViewById<RecyclerView>(R.id.looperListView)
            looperListView.layoutManager = LinearLayoutManager(context)
            mLooperListViewAdapter = LooperListViewAdapter(Collections.emptyList())
            looperListView.adapter = mLooperListViewAdapter

            // set up find loopers button
            val findLoopersButton = mSelectLooperDialogView.findViewById<Button>(R.id.findLoopersButton)
            findLoopersButton.setOnClickListener {
                triggerFindingLoopers()
            }

            // create dialog
            return builder.create()
        }

        private fun triggerFindingLoopers() {
            val looperService = (activity as MainActivity).mLooperService

            if (!looperService.isBtSupported()) {
                // bluetooth not supported by device
                showAlert("Sorry, your device doesn't support Bluetooth")
            } else if (!looperService.isBtEnabled()) {
                // bluetooth supported but not enabled, request enabling it
                val enableBtIntent = Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE)
                startActivityForResult(enableBtIntent, REQUEST_ENABLE_BT)
            } else {
                // bluetooth is supported and enabled, find loopers
                looperService.findLoopers()
            }

        }

        override fun onActivityResult(requestCode: Int, resultCode: Int, data: Intent?) {
            super.onActivityResult(
                    requestCode,
                    resultCode,
                    data
            )
            if(requestCode == REQUEST_ENABLE_BT && resultCode == Activity.RESULT_OK) {
                triggerFindingLoopers()
            } else {
                showAlert("Not much fun without Bluetooth though...")
            }
        }

        private fun showAlert(message: String) {
            val alertDialog = AlertDialog.Builder(context!!).create()
            alertDialog.setMessage(message)
            alertDialog.setButton(AlertDialog.BUTTON_NEUTRAL, "OK") { _, _ -> dialog.dismiss() }
            alertDialog.show()
        }

        override fun onDialogClosed(positiveResult: Boolean) {
            if (positiveResult) {
                val newValue = "selected looper name"
                (preference as LooperInUsePreference).update(newValue)
            }
        }

        override fun onDestroy() {
            LocalBroadcastManager.getInstance(context!!).unregisterReceiver(mReceiver)
            super.onDestroy()
        }
    }

    class LooperListViewAdapter(private var looperList: List<BluetoothDevice>) :
            RecyclerView.Adapter<LooperListViewAdapter.ViewHolder>() {

        override fun onBindViewHolder(holder: ViewHolder, position: Int) {
            // this implements showing at least 3 items in view even when looper list contains less items
            holder.itemTextView.text =
                when (looperList.size) {
                    // looper list is empty, show message on first item and leave other 2 blank
                    0 -> if (position == 0) "--no loopers found--" else ""
                    // looper list is not empty but has less than 3 items, show looper name for all items from looper list and leave others blank
                    1,2,3 -> if (position < looperList.size) looperList[position].name else ""
                    // looper list has 3 or more items, show names for all of them
                    else -> looperList[position].name
                }
        }

        override fun getItemCount(): Int {
            // the lowest possible number returned is 3 in order to show at least 3 items in list even when looper list contains less
            return if (looperList.size > 3) looperList.size else 3
        }

        class ViewHolder(itemView: View) : RecyclerView.ViewHolder(itemView) {
            var looperNameView: View = itemView
            var itemTextView = itemView.findViewById(R.id.item_text_view) as TextView
        }

        override fun onCreateViewHolder(parent: ViewGroup, viewType: Int): ViewHolder {
            val view = LayoutInflater.from(parent.context)
                    .inflate(R.layout.dialog_select_looper_list_item, parent, false)

            return ViewHolder(view)
        }

        fun updateLooperList(updatedLooperList: List<BluetoothDevice>) {
            looperList = updatedLooperList
        }

    }
}

