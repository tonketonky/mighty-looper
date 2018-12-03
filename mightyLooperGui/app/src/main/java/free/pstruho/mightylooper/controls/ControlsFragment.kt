package free.pstruho.mightylooper.controls

import android.content.*
import android.os.Bundle
import android.os.IBinder
import android.support.v4.app.Fragment
import android.support.v4.content.LocalBroadcastManager
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.GridLayout
import free.pstruho.mightylooper.R
import free.pstruho.mightylooper.service.LooperService
import free.pstruho.mightylooper.utils.buildMessage

class ControlsFragment : Fragment() {

    private lateinit var mLooperService: LooperService
    private var mBound = false

    private val mReceiver = object : BroadcastReceiver() {
        override fun onReceive(context: Context, intent: Intent) {
            val action = intent.action
            when (action) {

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

    private val mHandleCommand = fun(command: String, args: List<String>) {
        mLooperService.write(buildMessage(command, args))
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        // bind looper service
        val intent = Intent(requireContext(), LooperService::class.java)
        requireActivity().bindService(intent, mConnection, Context.BIND_AUTO_CREATE)

        // register broadcast receiver
        val filter = IntentFilter()
        //filter.addAction(ACT_SET_TEMPO)
        LocalBroadcastManager.getInstance(requireContext()).registerReceiver(mReceiver, filter)
    }

    override fun onCreateView(inflater: LayoutInflater, container: ViewGroup?,
                              savedInstanceState: Bundle?): View? {

        val controlsLayout =  inflater.inflate(R.layout.fragment_controls, container, false) as GridLayout

        val controlsT1Ch1 = controlsLayout.findViewById<TrackControls>(R.id.controlsT1Ch1)
        controlsT1Ch1.mHandleCommand = mHandleCommand
        val controlsT2Ch1 = controlsLayout.findViewById<TrackControls>(R.id.controlsT2Ch1)
        controlsT2Ch1.mHandleCommand = mHandleCommand
        val controlsT3Ch1 = controlsLayout.findViewById<TrackControls>(R.id.controlsT3Ch1)
        controlsT3Ch1.mHandleCommand = mHandleCommand
        val controlsT4Ch1 = controlsLayout.findViewById<TrackControls>(R.id.controlsT4Ch1)
        controlsT4Ch1.mHandleCommand = mHandleCommand

        val controlsT1Ch2 = controlsLayout.findViewById<TrackControls>(R.id.controlsT1Ch2)
        controlsT1Ch2.mHandleCommand = mHandleCommand
        val controlsT2Ch2 = controlsLayout.findViewById<TrackControls>(R.id.controlsT2Ch2)
        controlsT2Ch2.mHandleCommand = mHandleCommand
        val controlsT3Ch2 = controlsLayout.findViewById<TrackControls>(R.id.controlsT3Ch2)
        controlsT3Ch2.mHandleCommand = mHandleCommand
        val controlsT4Ch2 = controlsLayout.findViewById<TrackControls>(R.id.controlsT4Ch2)
        controlsT4Ch2.mHandleCommand = mHandleCommand

        return controlsLayout
    }

    override fun onDestroy() {
        LocalBroadcastManager.getInstance(requireContext()).unregisterReceiver(mReceiver)
        requireActivity().unbindService(mConnection)
        super.onDestroy()
    }

}
