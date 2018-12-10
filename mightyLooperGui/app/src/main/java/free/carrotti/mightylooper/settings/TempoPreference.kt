package free.carrotti.mightylooper.settings

import android.content.Context
import android.content.res.TypedArray
import android.support.v7.preference.DialogPreference
import android.support.v7.preference.PreferenceDialogFragmentCompat
import android.util.AttributeSet
import android.view.View
import android.view.ViewGroup
import android.widget.FrameLayout
import android.widget.NumberPicker

private const val DEFAULT_TEMPO = 90
private const val MAX_VALUE = 200
private const val MIN_VALUE = 1
private const val WRAP_SELECTOR_WHEEL = true

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

    class TempoDialog : PreferenceDialogFragmentCompat() {

        private lateinit var mNumberPicker: NumberPicker

        override fun onCreateDialogView(context: Context): View {
            val layoutParams = FrameLayout.LayoutParams(
                    ViewGroup.LayoutParams.WRAP_CONTENT,
                    ViewGroup.LayoutParams.WRAP_CONTENT
            )
            mNumberPicker = NumberPicker(context)
            mNumberPicker.layoutParams = layoutParams

            val dialogView = FrameLayout(context)
            dialogView.addView(mNumberPicker)
            return dialogView
        }

        override fun onBindDialogView(view: View?) {
            super.onBindDialogView(view)
            mNumberPicker.minValue = MIN_VALUE
            mNumberPicker.maxValue = MAX_VALUE
            mNumberPicker.wrapSelectorWheel = WRAP_SELECTOR_WHEEL
            mNumberPicker.value = (preference as TempoPreference).mTempo
        }

        override fun onDialogClosed(positiveResult: Boolean) {
            if (positiveResult) {
                mNumberPicker.clearFocus()
                val newValue = mNumberPicker.value
                (preference as TempoPreference).update(newValue)
            }
        }
    }
}
