package free.pstruho.mightylooper.utils

// UUID of looper bluetooth server
const val SPP_UUID = "1e0ca4ea-299d-4335-93eb-27fcfe7fa848"

// tags for logger
const val TAG_LOOPER_SERVICE = "LOOPER SERVICE"
const val TAG_CONTROLS = "CONTROLS"

// actions used in broadcasts
const val ACT_UPDATE_LOOPER_LIST = "update_looper_list"
const val ACT_SET_TEMPO = "set_tempo"

// commands sent to looper
const val CMD_SET_TEMPO = "set_tempo"
const val CMD_REC_FLAG = "rec_flag"
const val CMD_REC_CANCEL = "rec_cancel"
const val CMD_REC_STOP = "rec_stop"
const val CMD_TRACK_FLAG_SWITCH_LOOPING = "trck_flag_switch_looping"
const val CMD_TRACK_SWITCH_MUTE = "trck_switch_mute"
const val CMD_TRACK_FLAG_SWAP_VERSIONS = "trck_flag_swap_versions"

// message utils constants
const val START_OF_COMMAND = "[cmd/"
const val END_OF_COMMAND = "]"
const val ARG_PREFIX = "arg"
const val STR_ARG_SIGN = "$"
const val NUM_ARG_SIGN = "#"
