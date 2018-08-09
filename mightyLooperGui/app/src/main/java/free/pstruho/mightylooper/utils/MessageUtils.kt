package free.pstruho.mightylooper.utils

import android.content.Intent

/*  Messages protocol
    #################

    Messages received from looper consist of command followed by slash-separated list of arguments
    example: "command/arg1/arg2/arg3"

    Messages consist of event code followed by slash-separated list of arguments, all enclosed in square brackets.
    Each argument is prefixed with # for numeric values or $ for string values.
    example [event_code/#666/$hellYeah]
 */

fun buildSetTempoMessage(tempo: Int): String {
    return "[$EVT_CD_SET_TEMPO/#$tempo]"
}

fun getIntentFromMessage(msg: String): Intent? {
    val literals = msg.split('/')

    return when (literals[0]) {
        MSG_CMD_SET_TEMPO -> {
            val intent = Intent()
            intent.action = ACT_SET_TEMPO
            intent.putExtra(ACT_ARG_TEMPO, literals[1].toInt())
            intent
        }
        else -> null
    }
}