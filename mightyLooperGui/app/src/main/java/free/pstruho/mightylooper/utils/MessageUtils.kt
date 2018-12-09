package free.pstruho.mightylooper.utils

import android.content.Intent

/*  Messages protocol
    #################

    Messages consist of message category (evt/cmd), message code (command or event code) followed by slash-separated list of arguments, all enclosed in square brackets.
    Each argument is prefixed with # for numeric values or $ for string values.
    example "[cmd/msg_code/#666/$hellYeah]"
 */

fun buildMessage(command: String, args: List<Any>): String {
    val argsStr = args.joinToString(separator = MESSAGE_LITERALS_DELIMITER) { arg -> if (arg is String) "$MESSAGE_STR_ARG_SIGN$arg" else "$MESSAGE_NUM_ARG_SIGN$arg" }
    return "$MESSAGE_BEGINNING$command$MESSAGE_LITERALS_DELIMITER$argsStr$MESSAGE_END"
}

fun getIntentFromMessage(msg: String): Intent? {

    // look for first group of message format regex which contains message body (between "[cmd/" and "]")
    val msgBody = MESSAGE_FORMAT_REGEX.toRegex().find(msg)?.groups?.get(1)?.value

    // if message body found parse command and arguments, build and return intent
    return msgBody?.let {

        val literals = it.split(MESSAGE_LITERALS_DELIMITER)

        val intent = Intent()
        // set first literal as action
        intent.action = literals[0]

        // iterate over the rest of literals
        for (i in 1 until literals.size step 1) {
            // based on first char ($ or #) set value in corresponding type as argument with name INTENT_ARG_PREFIX + position
            when (literals[i].first().toString()) {
                MESSAGE_STR_ARG_SIGN -> intent.putExtra("$INTENT_ARG_PREFIX$i", literals[i].substring(1))
                MESSAGE_NUM_ARG_SIGN -> intent.putExtra("$INTENT_ARG_PREFIX$i", literals[i].substring(1).toInt())
            }
        }
        intent
    }
}