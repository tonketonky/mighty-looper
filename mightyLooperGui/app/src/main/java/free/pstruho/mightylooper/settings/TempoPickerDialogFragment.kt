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

class TempoPickerDialogFragment : PreferenceDialogFragmentCompat() {

    private lateinit var picker: NumberPicker

    override fun onCreateDialogView(context: Context): View {
        val layoutParams = FrameLayout.LayoutParams(
                ViewGroup.LayoutParams.WRAP_CONTENT,
                ViewGroup.LayoutParams.WRAP_CONTENT
        )
        picker = NumberPicker(context)
        picker.layoutParams = layoutParams

        val dialogView = FrameLayout(context)
        dialogView.addView(picker)
        return dialogView
    }

    override fun onBindDialogView(view: View?) {
        super.onBindDialogView(view)
        picker.minValue = MIN_VALUE
        picker.maxValue = MAX_VALUE
        picker.wrapSelectorWheel = WRAP_SELECTOR_WHEEL
        picker.value = (preference as TempoPickerPreference).tempo
    }

    override fun onDialogClosed(positiveResult: Boolean) {
        if (positiveResult) {
            picker.clearFocus()
            val newValue = picker.value
            (preference as TempoPickerPreference).update(newValue)
        }
    }
}