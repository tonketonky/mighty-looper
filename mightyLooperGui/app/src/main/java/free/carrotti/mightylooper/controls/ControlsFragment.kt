package free.carrotti.mightylooper.controls

import android.content.*
import android.os.Bundle
import android.os.IBinder
import android.support.v4.app.Fragment
import android.support.v4.content.LocalBroadcastManager
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.GridLayout
import free.carrotti.mightylooper.R
import free.carrotti.mightylooper.service.LooperService
import free.carrotti.mightylooper.utils.*

class ControlsFragment : Fragment() {

    private lateinit var mLooperService: LooperService
    private val mTrackControlsList = mutableListOf<TrackControls>()
    private var mBound = false

    private val mReceiver = object : BroadcastReceiver() {
        override fun onReceive(context: Context, intent: Intent) {
            val action = intent.action
            when (action) {
                CMD_RECORDING_FLAGGED -> getTrackControls(intent)?.let { it.handleFlaggedForRecording() }
                CMD_RECORDING_UNFLAGGED -> getTrackControls(intent)?.let { it.handleUnflaggedForRecording() }
                CMD_RECORDING_STARTED -> getTrackControls(intent)?.let { it.handleStartedRecording() }
                CMD_RECORDING_STOPPED -> getTrackControls(intent)?.let { it.handleStoppedRecording() }
                CMD_TRACK_SWITCH_LOOPING_FLAGGED -> getTrackControls(intent)?.let { it.handleFlaggedForLooping() }
                CMD_TRACK_SWITCH_LOOPING_UNFLAGGED -> getTrackControls(intent)?.let { it.handleUnflaggedForLooping() }
                CMD_TRACK_LOOPING_STARTED -> getTrackControls(intent)?.let { it.handleStartedLooping() }
                CMD_TRACK_LOOPING_STOPPED -> getTrackControls(intent)?.let { it.handleStoppedLooping() }
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

    private val mWriteToLooperService = fun(command: String, args: List<String>) {
        mLooperService.write(buildMessage(command, args))
    }

    private fun getTrackControls(intent: Intent): TrackControls? {
        return mTrackControlsList.find { item ->
            item.mChannel == intent.getStringExtra("${INTENT_ARG_PREFIX}1")
                    && item.mTrack == intent.getStringExtra("${INTENT_ARG_PREFIX}2") }
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        // bind looper service
        val intent = Intent(requireContext(), LooperService::class.java)
        requireActivity().bindService(intent, mConnection, Context.BIND_AUTO_CREATE)

        // register broadcast receiver
        val filter = IntentFilter()
        filter.addAction(CMD_RECORDING_FLAGGED)
        filter.addAction(CMD_RECORDING_UNFLAGGED)
        filter.addAction(CMD_RECORDING_STARTED)
        filter.addAction(CMD_RECORDING_STOPPED)
        filter.addAction(CMD_TRACK_SWITCH_LOOPING_FLAGGED)
        filter.addAction(CMD_TRACK_SWITCH_LOOPING_UNFLAGGED)
        filter.addAction(CMD_TRACK_LOOPING_STARTED)
        filter.addAction(CMD_TRACK_LOOPING_STOPPED)
        LocalBroadcastManager.getInstance(requireContext()).registerReceiver(mReceiver, filter)
    }

    override fun onCreateView(inflater: LayoutInflater, container: ViewGroup?,
                              savedInstanceState: Bundle?): View? {

        val controlsLayout =  inflater.inflate(R.layout.fragment_controls, container, false) as GridLayout

        mTrackControlsList.add(controlsLayout.findViewById(R.id.controlsT1Ch1))
        mTrackControlsList.add(controlsLayout.findViewById(R.id.controlsT2Ch1))
        mTrackControlsList.add(controlsLayout.findViewById(R.id.controlsT3Ch1))
        mTrackControlsList.add(controlsLayout.findViewById(R.id.controlsT4Ch1))

        mTrackControlsList.add(controlsLayout.findViewById(R.id.controlsT1Ch2))
        mTrackControlsList.add(controlsLayout.findViewById(R.id.controlsT2Ch2))
        mTrackControlsList.add(controlsLayout.findViewById(R.id.controlsT3Ch2))
        mTrackControlsList.add(controlsLayout.findViewById(R.id.controlsT4Ch2))

        mTrackControlsList.forEach { trackControls -> trackControls.mSendToLooperService = mWriteToLooperService }

        return controlsLayout
    }

    override fun onDestroy() {
        LocalBroadcastManager.getInstance(requireContext()).unregisterReceiver(mReceiver)
        requireActivity().unbindService(mConnection)
        super.onDestroy()
    }

}
