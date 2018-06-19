package free.pstruho.mightylooper.settings

import android.content.Context
import android.content.res.TypedArray
import android.support.v7.preference.DialogPreference
import android.util.AttributeSet

class TempoPickerPreference(context: Context, attrs: AttributeSet) : DialogPreference(context, attrs) {

    private val default = 90
    var tempo = 0

    override fun onGetDefaultValue(a: TypedArray, index: Int): Any {
        return a.getInt(index, default)
    }

    override fun onSetInitialValue(restorePersistedValue: Boolean, defaultValue: Any?) {
        update( if (restorePersistedValue) getPersistedInt(default) else defaultValue as Int )
    }

    fun update(tempo: Int) {
        persistInt(tempo)
        this.tempo = tempo
        this.summary = "$tempo bpm"
    }
}
