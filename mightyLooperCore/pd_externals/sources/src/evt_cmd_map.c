#include "m_pd.h"
#include "search.h"
#include "stdlib.h"
#include "string.h"
#include "stdio.h"

/* data structs and vars for map implementation - begin */
typedef struct {
    char *key;
    char *value;
} evt_cmd_map;

void *evt_cmd_map_root;
evt_cmd_map *evt_cmd_map_find;
/* data structs and vars for map implementation - end */

/**
 * Comparator function for map implementation
 */
int evt_cmd_map_compar(const void *l, const void *r) {
    const evt_cmd_map *lm = l;
    const evt_cmd_map *lr = r;
    return strcmp(lm->key, lr->key);
}

/**
 * Adds event code-command pair to map
 *
 * @param evt pointer to event code string
 * @param cmd pointer to command string
 */
void add_evt_cmd_pair(char *evt, char *cmd) {
    evt_cmd_map *pair= malloc(sizeof(evt_cmd_map));
    pair->key = evt;
    pair->value = cmd;
    tsearch(pair, &evt_cmd_map_root, evt_cmd_map_compar);
}

/**
 * Initializes event code-command map with event code-command pairs for record and play buttons
 */
void init_evt_cmd_map(void) {
    evt_cmd_map_root = 0;
    evt_cmd_map_find = malloc(sizeof(evt_cmd_map));

    /* event codes from record buttons */

    // single press for track - recording ON/OFF
    add_evt_cmd_pair("r_sp_t_r=0", "flag_recording");
    add_evt_cmd_pair("r_sp_t_r=1", "stop_recording");
    // long press for track - recording ON/OFF
    //add_evt_cmd_pair("r_lp_t_r=0", "flag_recording_both_phrases");
    //add_evt_cmd_pair("r_lp_t_r=1", "cancel_recording");

    /* event codes from play buttons */

    // single press for track
    add_evt_cmd_pair("p_sp_t", "flag_switch_looping");
    // double press for track
    //add_evt_cmd_pair("p_dp_t", "mute_track");
    // long press for track
    add_evt_cmd_pair("p_lp_t", "flag_swap_versions");
    // single press for channel
    //add_evt_cmd_pair("p_sp_ch", "flag_channel_switch_looping");
    // long press for channel
    //add_evt_cmd_pair("p_dp_ch", "mute_channel");

    /* event codes from click settings */

    // set tempo
    add_evt_cmd_pair("c_t", "set_tempo");
    // set time signature
    add_evt_cmd_pair("c_ts", "set_time_signature");
}

char *get_cmd_for_evt(char *key) {
    evt_cmd_map_find->key = key;
    void *r = tfind(evt_cmd_map_find, &evt_cmd_map_root, evt_cmd_map_compar);
    if(r == NULL) {
        return NULL;
    } else {
        return (*(evt_cmd_map**)r)->value;
    }
}
