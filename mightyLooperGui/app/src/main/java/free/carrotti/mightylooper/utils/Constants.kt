package free.carrotti.mightylooper.utils

// connection constants
const val SPP_UUID = "1e0ca4ea-299d-4335-93eb-27fcfe7fa848"

// internal actions
const val ACT_UPDATE_LOOPER_LIST = "update_looper_list"

// broadcast constants
const val INTENT_ARG_PREFIX = "arg"

// message constants
const val MESSAGE_LITERALS_DELIMITER = "/"
const val MESSAGE_BEGINNING = "[cmd$MESSAGE_LITERALS_DELIMITER"
const val MESSAGE_END = "]"
const val MESSAGE_STR_ARG_SIGN = "$"
const val MESSAGE_NUM_ARG_SIGN = "#"
const val MESSAGE_FORMAT_REGEX = "\\$MESSAGE_BEGINNING([^\\]]+)\\$MESSAGE_END"

// commands sent to/received from looper
const val CMD_SET_TEMPO = "set_tempo"

const val CMD_RECORDING_FLAG = "rec_flag"
const val CMD_RECORDING_CANCEL = "rec_cancel"
const val CMD_RECORDING_STOP = "rec_stop"

const val CMD_RECORDING_FLAGGED = "recording_flagged"
const val CMD_RECORDING_UNFLAGGED = "recording_unflagged"
const val CMD_RECORDING_STARTED = "recording_started"
const val CMD_RECORDING_STOPPED = "recording_stopped"

const val CMD_TRACK_SWITCH_LOOPING_FLAGGED = "track_switch_looping_flagged"
const val CMD_TRACK_SWITCH_LOOPING_UNFLAGGED = "track_switch_looping_unflagged"

const val CMD_TRACK_LOOPING_STARTED = "track_looping_started"
const val CMD_TRACK_LOOPING_STOPPED = "track_looping_stopped"

const val CMD_TRACK_FLAG_SWITCH_LOOPING = "trck_flag_switch_looping"
const val CMD_TRACK_SWITCH_MUTE = "trck_switch_mute"
const val CMD_TRACK_FLAG_SWAP_VERSIONS = "trck_flag_swap_versions"

const val CMD_SYNC = "sync"
const val CMD_SHUTDOWN = "shutdown"

// logger tags
const val TAG_LOOPER_SERVICE = "LOOPER SERVICE"
const val TAG_CONTROLS = "CONTROLS"

// log messages
const val LOG_CLOSING_SOCKET_ERROR = "Could not close the connect socket"
const val LOG_COULD_NOT_CLOSE_CLIENT_SOCKET = "Could not close the client socket"
const val LOG_COULD_NOT_CONNECT_TO_DEVICE = "Could not connect to device, closing socket"
const val LOG_CREATING_IN_STREAM_ERROR = "Error occurred when creating input stream"
const val LOG_CREATING_OUT_STREAM_ERROR = "Error occurred when creating output stream"
const val LOG_CREATING_SOCKET_FAILED = "Creating socket failed"
const val LOG_INPUT_STREAM_DISCONNECTED = "Input stream was disconnected"
const val LOG_RECEIVED_MESSAGE = "Received message: "
const val LOG_SENDING_DATA_ERROR = "Error occurred when sending data"
const val LOG_SOCKET_CLOSED_DUE_TO_EXCEPTION = "Socket closed due to exception"
const val LOG_SOCKET_CLOSED_ON_THREAD_CANCEL = "Socket closed on thread cancel"
const val LOG_SOCKET_CONNECTED = "Socket connected"
const val LOG_SOCKET_CREATED = "Socket created"
