package free.pstruho.mightylooper.utils

import android.content.Intent

/*  Messages protocol
    #################

    Messages consist of message category (evt/cmd), message code (command or event code) followed by slash-separated list of arguments, all enclosed in square brackets.
    Each argument is prefixed with # for numeric values or $ for string values.
    example "[evt/msg_code/#666/$hellYeah]"
 */

fun buildMessage(command: String, args: List<Any>): String {
    val argsStr = args.joinToString(separator = "/") { arg -> if (arg is String) "$STR_ARG_SIGN$arg" else "$NUM_ARG_SIGN$arg" }
    return "[cmd/$command/$argsStr]"
}

fun getIntentFromMessage(msg: String): Intent? {

    if (msg.startsWith(START_OF_COMMAND) and msg.endsWith(END_OF_COMMAND)) {
        // is valid message
        val msgBody = msg.drop(START_OF_COMMAND.length).dropLast(END_OF_COMMAND.length)
        val literals = msgBody.split('/')

        val intent = Intent()
        // set first literal as action
        intent.action = literals[0]

        // iterate over the rest of literals
        for(i in 1 until literals.size step 1) {
            // based on first char ($ or #) set value in corresponding type as argument with name ARG_PREFIX + position
            when (literals[i].first().toString()) {
                STR_ARG_SIGN -> intent.putExtra("$ARG_PREFIX$i", literals[i].substring(1))
                NUM_ARG_SIGN -> intent.putExtra("$ARG_PREFIX$i", literals[i].substring(1).toInt())
            }
        }
        return intent
    } else {
        // is NOT valid message
        return null
    }

}