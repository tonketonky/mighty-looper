package free.pstruho.mightylooper.service

import android.app.Service
import android.bluetooth.BluetoothAdapter
import android.content.Intent
import android.os.IBinder
import android.bluetooth.BluetoothDevice
import android.content.BroadcastReceiver
import android.content.Context
import android.content.IntentFilter
import android.os.Binder
import java.util.*
import android.support.v4.content.LocalBroadcastManager

const val UPDATED_LOOPER_LIST = "updated_device_list"

class LooperService : Service() {

    private val mDeviceList = ArrayList<BluetoothDevice>()

    private val mBluetoothAdapter = BluetoothAdapter.getDefaultAdapter()
    private lateinit var mLocalBroadcastManager: LocalBroadcastManager

    private val mBinder = LocalBinder()

    private val mReceiver = object : BroadcastReceiver() {
        override fun onReceive(context: Context, intent: Intent) {
            val action = intent.action
            if (BluetoothDevice.ACTION_FOUND == action) {
                val device = intent.getParcelableExtra<BluetoothDevice>(BluetoothDevice.EXTRA_DEVICE)
                mDeviceList.add(device)

                // notify about updating looper list
                val updateLooperListIntent = Intent()
                updateLooperListIntent.action = UPDATED_LOOPER_LIST
                mLocalBroadcastManager.sendBroadcast(updateLooperListIntent)
            }
        }
    }

    override fun onCreate() {
        super.onCreate()
        val filter = IntentFilter(BluetoothDevice.ACTION_FOUND)
        registerReceiver(mReceiver, filter)
        mLocalBroadcastManager = LocalBroadcastManager.getInstance(this)
    }

    override fun onBind(intent: Intent?): IBinder? {
        return mBinder
    }

    fun getDeviceList() : List<BluetoothDevice> {
         return mDeviceList
    }

    fun findLoopers() {
        mBluetoothAdapter.startDiscovery()
    }

    fun isBtSupported(): Boolean {
        return mBluetoothAdapter != null
    }

    fun isBtEnabled(): Boolean {
        return mBluetoothAdapter.isEnabled
    }

    inner class LocalBinder : Binder() {
        fun getService(): LooperService {
            return this@LooperService
        }
    }

    override fun onDestroy() {
        unregisterReceiver(mReceiver)
        super.onDestroy()
    }

}