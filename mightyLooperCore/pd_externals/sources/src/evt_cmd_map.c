#include "m_pd.h"
#include "helpers_and_types.h"
#include "search.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

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
    add_evt_cmd_pair(strcat(malloc(sizeof(EVT_REC_TRACK_BUTTON_SP) + 4), "_r=0"), CMD_REC_FLAG);
    add_evt_cmd_pair(strcat(malloc(sizeof(EVT_REC_TRACK_BUTTON_SP) + 4), "_r=1"), CMD_REC_STOP);
    // long press for track - recording ON/OFF
    //add_evt_cmd_pair(strcat(EVT_REC_TRACK_BUTTON_LP, "_r=0"), CMD_REC_FLAG_BOTH_PHRASES);
    //add_evt_cmd_pair(strcat(EVT_REC_TRACK_BUTTON_LP, "_r=1"), CMD_REC_CANCEL);

    /* event codes from play buttons */

    // single press for track
    add_evt_cmd_pair(EVT_PLAY_TRACK_BUTTON_SP, CMD_TRACK_FLAG_SWITCH_LOOPING);
    // double press for track
    //add_evt_cmd_pair(EVT_PLAY_TRACK_BUTTON_DP, CMD_TRACK_SWITCH_MUTE);
    // long press for track
    add_evt_cmd_pair(EVT_PLAY_TRACK_BUTTON_LP, CMD_TRACK_FLAG_SWAP_VERSIONS);
    // single press for channel
    //add_evt_cmd_pair(EVT_PLAY_CHANNEL_BUTTON_SP, CMD_FLAG_CHANNEL_SWITCH_LOOPING);
    // long press for channel
    //add_evt_cmd_pair(EVT_PLAY_CHANNEL_BUTTON_DP, CMD_CHANNEL_SWITCH_MUTE);
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
