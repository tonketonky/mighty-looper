package free.pstruho.mightylooper.controls

import android.content.Context
import android.util.AttributeSet
import android.widget.Button
import android.widget.GridLayout
import free.pstruho.mightylooper.R
import free.pstruho.mightylooper.utils.CMD_REC_FLAG
import free.pstruho.mightylooper.utils.CMD_TRACK_FLAG_SWITCH_LOOPING
import free.pstruho.mightylooper.utils.CMD_TRACK_SWITCH_MUTE

class TrackControls constructor(context: Context, attrs: AttributeSet) : GridLayout(context, attrs) {

    private val mLoopButton: Button
    private val mRecButton: Button
    private val mMuteButton: Button

    private val mTrack: String
    private val mChannel: String

    lateinit var mHandleCommand: (command: String, args: List<String>) -> Unit


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

        mLoopButton.setOnClickListener { mHandleCommand(CMD_TRACK_FLAG_SWITCH_LOOPING, listOf(mChannel, mTrack)) }
        mRecButton.setOnClickListener { mHandleCommand(CMD_REC_FLAG, listOf(mChannel, mTrack)) }
        mMuteButton.setOnClickListener { mHandleCommand(CMD_TRACK_SWITCH_MUTE, listOf(mChannel, mTrack)) }
    }


}