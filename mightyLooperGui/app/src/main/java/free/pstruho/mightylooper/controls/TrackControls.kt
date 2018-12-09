package free.pstruho.mightylooper.controls

import android.content.Context
import android.util.AttributeSet
import android.widget.Button
import android.widget.GridLayout
import android.support.v4.content.ContextCompat
import free.pstruho.mightylooper.R
import free.pstruho.mightylooper.utils.CMD_RECORDING_FLAG
import free.pstruho.mightylooper.utils.CMD_RECORDING_STOP
import free.pstruho.mightylooper.utils.CMD_TRACK_FLAG_SWITCH_LOOPING
import free.pstruho.mightylooper.utils.CMD_TRACK_SWITCH_MUTE

class TrackControls constructor(context: Context, attrs: AttributeSet) : GridLayout(context, attrs) {

    private val mLoopButton: Button
    private val mRecButton: Button
    private val mMuteButton: Button

    val mTrack: String
    val mChannel: String

    lateinit var mSendToLooperService: (command: String, args: List<String>) -> Unit

    init {

        inflate(context, R.layout.track_controls, this)

        val a = getContext().theme.obtainStyledAttributes(attrs, R.styleable.TrackControls, 0, 0)
        // Extract custom attributes into member variables
        try {
            mTrack = a.getString(R.styleable.TrackControls_trackId)
            mChannel = a.getString(R.styleable.TrackControls_channelId)
        } finally {
            // TypedArray objects are shared and must be recycled.
            a.recycle()
        }

        mLoopButton = findViewById(R.id.loopButton)
        mRecButton = findViewById(R.id.recButton)
        mMuteButton = findViewById(R.id.muteButton)

        mLoopButton.setOnClickListener { mSendToLooperService(CMD_TRACK_FLAG_SWITCH_LOOPING, listOf(mChannel, mTrack)) }
        mRecButton.setOnClickListener { mSendToLooperService(CMD_RECORDING_FLAG, listOf(mChannel, mTrack)) }
        mMuteButton.setOnClickListener { mSendToLooperService(CMD_TRACK_SWITCH_MUTE, listOf(mChannel, mTrack)) }
    }

    fun handleFlaggedForRecording() {
        setIcon(mRecButton, R.drawable.red_flag_crossed_icon)
    }

    fun handleUnflaggedForRecording() {
        setIcon(mRecButton, R.drawable.red_flag_icon)
    }

    fun handleStartedRecording() {
        mRecButton.setOnClickListener { mSendToLooperService(CMD_RECORDING_STOP, listOf(mChannel, mTrack)) }
        setIcon(mRecButton, R.drawable.stop_icon)
    }

    fun handleStoppedRecording() {
        mRecButton.setOnClickListener { mSendToLooperService(CMD_RECORDING_FLAG, listOf(mChannel, mTrack)) }
        setIcon(mRecButton, R.drawable.red_flag_icon)
    }

    fun handleFlaggedForLooping() {
        setIcon(mLoopButton, R.drawable.green_flag_crossed_icon)
    }

    fun handleUnflaggedForLooping() {
        setIcon(mLoopButton, R.drawable.green_flag_icon)
    }

    fun handleStartedLooping() {
        setIcon(mLoopButton, R.drawable.green_flag_icon)
    }

    fun handleStoppedLooping() {
        setIcon(mLoopButton, R.drawable.green_flag_icon)
    }

    private fun setIcon(button: Button, drawableResource: Int) {
        button.setCompoundDrawablesWithIntrinsicBounds(ContextCompat.getDrawable(context, drawableResource), null, null, null)
    }
}