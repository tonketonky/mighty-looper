package free.pstruho.mightylooper.service

import android.app.Service
import android.bluetooth.BluetoothAdapter
import android.content.Intent
import android.bluetooth.BluetoothDevice
import android.content.BroadcastReceiver
import android.content.Context
import android.content.IntentFilter
import java.util.*
import android.support.v4.content.LocalBroadcastManager
import android.bluetooth.BluetoothSocket
import android.os.*
import android.util.Log
import free.pstruho.mightylooper.constants.TAG_LOOPER_SERVICE
import free.pstruho.mightylooper.constants.ACTION_UPDATED_LOOPER_LIST
import java.io.*

class LooperService : Service() {

    private val mDeviceList = ArrayList<BluetoothDevice>()

    private val mBluetoothAdapter = BluetoothAdapter.getDefaultAdapter()
    private lateinit var mLocalBroadcastManager: LocalBroadcastManager

    private val mBinder = LocalBinder()

    private lateinit var mConnectThread: ConnectThread
    private lateinit var mConnectedThread: ConnectedThread

    private val mReceiver = object : BroadcastReceiver() {
        override fun onReceive(context: Context, intent: Intent) {
            val action = intent.action
            if (BluetoothDevice.ACTION_FOUND == action) {
                val device = intent.getParcelableExtra<BluetoothDevice>(BluetoothDevice.EXTRA_DEVICE)

                // if found device is not already in the list add it
                if(mDeviceList.none { item -> device.address == item.address }) {
                    mDeviceList.add(device)
                }

                // notify about updating looper list
                val updateLooperListIntent = Intent()
                updateLooperListIntent.action = ACTION_UPDATED_LOOPER_LIST
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

    override fun onStartCommand(intent: Intent?, flags: Int, startId: Int): Int {
        return START_STICKY
    }

    override fun onBind(intent: Intent?): IBinder? {
        return mBinder
    }

    fun getDeviceList() : List<BluetoothDevice> {
         return mDeviceList
    }

    fun write(data: String) {
        mConnectedThread.write(data.toByteArray())
    }

    fun connectLooper(address: String) {
        mBluetoothAdapter.cancelDiscovery()

        val device = mDeviceList.first { it.address == address }

        mConnectThread = ConnectThread(device)
        mConnectThread.start()
    }

    private fun manageMyConnectedSocket(socket: BluetoothSocket) {
        mConnectedThread = ConnectedThread(socket)
        mConnectedThread.start()
    }

    fun findLoopers() {
        if(!mBluetoothAdapter.isDiscovering) {
            mBluetoothAdapter.startDiscovery()

            // stop discovering after 10 seconds
            Handler().postDelayed({
                mBluetoothAdapter.cancelDiscovery()
            }, 10000)
        }
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
        if(mConnectThread != null) {
            mConnectThread.cancel()
        }
        if(mConnectedThread != null) {
            mConnectedThread.cancel()
        }
        mBluetoothAdapter.cancelDiscovery()
        unregisterReceiver(mReceiver)
        super.onDestroy()
    }

    private inner class ConnectThread(mDevice: BluetoothDevice) : Thread() {
        private val mSocket: BluetoothSocket?

        init {
            var tmp: BluetoothSocket? = null

            try {
                val m = mDevice.javaClass.getMethod("createRfcommSocket", Int::class.javaPrimitiveType!!)
                tmp = m.invoke(mDevice, 22) as BluetoothSocket
                Log.i(TAG_LOOPER_SERVICE, "Socket created")
            } catch (e: IOException) {
                Log.e(TAG_LOOPER_SERVICE, "Socket's create() method failed", e)
            }

            mSocket = tmp
        }

        override fun run() {
            // Cancel discovery because it otherwise slows down the connection.
            mBluetoothAdapter.cancelDiscovery()

            try {
                mSocket!!.connect()
                Log.i(TAG_LOOPER_SERVICE, "Socket connected")
            } catch (connectException: IOException) {
                Log.e(TAG_LOOPER_SERVICE, "Could not connect to device, closing socket", connectException)
                try {
                    mSocket!!.close()
                    connectException.printStackTrace()
                    Log.i(TAG_LOOPER_SERVICE, "Socket closed due to exception")
                } catch (closeException: IOException) {
                    Log.e(TAG_LOOPER_SERVICE, "Could not close the client socket", closeException)
                }

                return
            }
            manageMyConnectedSocket(mSocket)
        }

        fun cancel() {
            try {
                mSocket!!.close()
                Log.i(TAG_LOOPER_SERVICE, "Socket closed due to cancel")
            } catch (e: IOException) {
                Log.e(TAG_LOOPER_SERVICE, "Could not close the client socket", e)
            }
        }
    }

    private inner class ConnectedThread(private val mSocket: BluetoothSocket) : Thread() {
        private val mInStream: InputStream?
        private val mOutStream: OutputStream?

        init {
            var tmpIn: InputStream? = null
            var tmpOut: OutputStream? = null

            try {
                tmpIn = mSocket.inputStream
            } catch (e: IOException) {
                Log.e(TAG_LOOPER_SERVICE, "Error occurred when creating input stream", e)
            }

            try {
                tmpOut = mSocket.outputStream
            } catch (e: IOException) {
                Log.e(TAG_LOOPER_SERVICE, "Error occurred when creating output stream", e)
            }

            mInStream = tmpIn
            mOutStream = tmpOut
        }
        val bufferedReader = BufferedReader(InputStreamReader(mInStream))
        override fun run() {
            val mBuffer = CharArray(128)
            var numBytes: Int // bytes returned from read()

            var data: String
            while (true) {
                try {
                    if(bufferedReader.ready()) {
                        numBytes = bufferedReader.read(mBuffer)
                        data = String(mBuffer, 0, numBytes)
                    }
                    // TODO: process data and do something
                } catch (e: IOException) {
                    Log.d(TAG_LOOPER_SERVICE, "Input stream was disconnected", e)
                    break
                }

            }
        }

        fun write(bytes: ByteArray) {
            try {
                mOutStream!!.write(bytes)
            } catch (e: IOException) {
                Log.e(TAG_LOOPER_SERVICE, "Error occurred when sending data", e)
            }

        }

        fun cancel() {
            try {
                mSocket.close()
            } catch (e: IOException) {
                Log.e(TAG_LOOPER_SERVICE, "Could not close the connect socket", e)
            }

        }
    }

}