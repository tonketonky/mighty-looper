#define PHRASE_1 gensym("p1")
#define PHRASE_2 gensym("p2")
#define VERSION_A gensym("a")
#define VERSION_B gensym("b")

/* message categories */
#define CAT_EVT "evt"
#define CAT_CMD "cmd"

/* event codes */
#define EVT_REC_TRACK_BUTTON_SP "rec_trck_sp"
#define EVT_REC_TRACK_BUTTON_DP "rec_trck__dp"
#define EVT_REC_TRACK_BUTTON_LP "rec_trck_lp"

#define EVT_PLAY_TRACK_BUTTON_SP "play_trck_sp"
#define EVT_PLAY_TRACK_BUTTON_DP "play_trck_dp"
#define EVT_PLAY_TRACK_BUTTON_LP "play_trck_lp"

#define EVT_PLAY_CHANNEL_BUTTON_SP "play_chnnl_sp"
#define EVT_PLAY_CHANNEL_BUTTON_DP "play_chnnl_dp"
#define EVT_PLAY_CHANNEL_BUTTON_LP "play_chnnl_lp"

/* commands */

// recorder
#define CMD_REC_FLAG "rec_flag"
#define CMD_REC_FLAG_BOTH_PHRASES "rec_flag_both_phrases"
#define CMD_REC_CANCEL "rec_cancel"
#define CMD_REC_STOP "rec_stop"

// player
#define CMD_CHANNEL_FLAG_SWITCH_LOOPING "chnnl_flag_switch_looping"
#define CMD_CHANNEL_SWITCH_MUTE "chnnl_switch_mute"

#define CMD_TRACK_SET_LOOPING "trck_set_looping"
#define CMD_TRACK_FLAG_SWITCH_LOOPING "trck_flag_switch_looping"
#define CMD_TRACK_FLAG_SWAP_VERSIONS "trck_flag_swap_versions"
#define CMD_TRACK_SWITCH_MUTE "trck_switch_mute"

// table allocator
#define CMD_ADD_TABLE "add_table"
#define CMD_FLAG_STOP_ALLOCATION "flag_stop_allocation"
#define CMD_SET_BEAT_NOTE_LENGTH "set_beat_note_length"
#define CMD_START_ALLOCATION "start_allocation"

// layer merger
#define CMD_FLAG_MERGING "flag_merging"

// track manager
#define CMD_SWAP_PHRASES "swap_phrases"

// click
#define CMD_SET_TIME_SIGNATURE "set_time_signature"
#define CMD_START_COUNTING_BEATS "start_counting_beats"
#define CMD_STOP_COUNTING_BEATS "stop_counting_beats"
#define CMD_SWITCH "switch"
#define CMD_SET_CURRENT_PHRASE "set_current_phrase"

// common
#define CMD_NEW_BAR "new_bar"
#define CMD_NEW_BEAT "new_beat"
#define CMD_NEW_CYCLE "new_cycle"
#define CMD_SET_UP_NEW_CYCLE "set_up_new_cycle"

#define CMD_RECORDING_STARTED "recording_started"
#define CMD_RECORDING_STOPPED "recording_stopped"

#define CMD_SET_TEMPO "set_tempo"
#define CMD_RESET "reset"
#define CMD_SYNC "sync"

/* command destinations */
#define DEST_CLICK "click"
#define DEST_PLAYER "player"
#define DEST_RECORDER "recorder"
#define DEST_TRACK_MANAGER "track_manager"

typedef enum{
    FREE_LENGTH,
    FIXED_LENGTH,
    NONE
} allocation_method;

t_symbol *get_opp_phrase(t_symbol *phrase);
t_symbol *get_opp_version(t_symbol *version);
void symb_2_string(t_symbol *symbol, char *string);
t_symbol *get_table_name_symb(t_symbol *phrase, t_symbol *channel, t_symbol *track, t_int layer, t_symbol *version);
