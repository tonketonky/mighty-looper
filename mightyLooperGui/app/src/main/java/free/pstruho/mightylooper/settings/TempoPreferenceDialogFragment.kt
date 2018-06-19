package free.pstruho.mightylooper.settings

import android.content.Context
import android.support.v7.preference.PreferenceDialogFragmentCompat
import android.view.View
import android.view.ViewGroup
import android.widget.FrameLayout
import android.widget.NumberPicker

private const val MAX_VALUE = 200
private const val MIN_VALUE = 1
private const val WRAP_SELECTOR_WHEEL = true

class TempoPreferenceDialogFragment : PreferenceDialogFragmentCompat() {

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