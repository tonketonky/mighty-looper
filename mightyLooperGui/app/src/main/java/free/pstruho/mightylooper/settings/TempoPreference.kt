package free.pstruho.mightylooper.settings

import android.content.Context
import android.content.res.TypedArray
import android.support.v7.preference.DialogPreference
import android.util.AttributeSet

private const val DEFAULT_TEMPO = 90

class TempoPreference(context: Context, attrs: AttributeSet) : DialogPreference(context, attrs) {

    var mTempo = 0

    override fun onGetDefaultValue(a: TypedArray, index: Int): Any {
        return a.getInt(index, DEFAULT_TEMPO)
    }

    override fun onSetInitialValue(restorePersistedValue: Boolean, defaultValue: Any?) {
        update( if (restorePersistedValue) getPersistedInt(DEFAULT_TEMPO) else defaultValue as Int )
    }

    fun update(tempo: Int) {
        persistInt(tempo)
        this.mTempo = tempo
        this.summary = "$tempo bpm"
    }
}
