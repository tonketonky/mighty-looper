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
import free.pstruho.mightylooper.utils.TAG_LOOPER_SERVICE
import free.pstruho.mightylooper.utils.ACT_UPDATE_LOOPER_LIST
import free.pstruho.mightylooper.utils.SPP_UUID
import free.pstruho.mightylooper.utils.getIntentFromMessage
import java.io.*
import java.lang.IllegalArgumentException

class LooperService : Service() {

    private val mDeviceList = ArrayList<BluetoothDevice>()

    private val mBluetoothAdapter = BluetoothAdapter.getDefaultAdapter()
    private lateinit var mLocalBroadcastManager: LocalBroadcastManager

    private val mBinder = LocalBinder()

    private lateinit var mConnectThread: ConnectThread
    private lateinit var mConnectedThread: ConnectedThread

    private var mConnected = false

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
                updateLooperListIntent.action = ACT_UPDATE_LOOPER_LIST

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

    private fun manageConnectedSocket(socket: BluetoothSocket) {
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
                tmp = mDevice.createRfcommSocketToServiceRecord(UUID.fromString(SPP_UUID))
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
                mConnected = true
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
            manageConnectedSocket(mSocket)
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

        //val bufferedReader = BufferedReader(InputStreamReader(mInStream))
        val bufferedReader = mInStream!!.bufferedReader()

        override fun run() {
            val mBuffer = CharArray(128)
            var numBytes: Int
            //var bytes = ByteArray(10)
            var msg: String

            while (mConnected) {
                try {
                    // for some reason read() method on InputStream get stuck in blocked state even though data is received
                    // this seems to be hardware specific issue, on some devices it might work
                    //numBytes = mInStream?.read(bytes) ?: 0

                    // Following is a workaround for issue with stuck read() method.
                    // Since BufferedReader is non-blocking read() method needs to be invoked in a loop
                    if(bufferedReader.ready()) {
                        numBytes = bufferedReader.read(mBuffer)
                        msg = String(mBuffer, 0, numBytes)
                        Log.d(TAG_LOOPER_SERVICE, "Received message: $msg")
                        getIntentFromMessage(msg)?.let { mLocalBroadcastManager.sendBroadcast(it) }
                    }
                    // for performance reasons the thread is paused in each iteration for 100ms
                    Thread.sleep(100)
                } catch (e: Exception) {
                    when(e) {
                        // TODO: investigate IllegalArgumentException here
                        is IOException, is IllegalArgumentException -> {
                            Log.d(TAG_LOOPER_SERVICE, "Input stream was disconnected", e)
                            mConnected = false
                        }
                        else -> throw e
                    }
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