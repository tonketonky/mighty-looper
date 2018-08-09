#include "m_pd.h"
#include "search.h"
#include "stdlib.h"
#include "string.h"
#include "stdio.h"

/* data structs and vars for map implementation - begin */
typedef struct {
	char *key;
	char *value;
} cmd_dest_map;

void *cmd_dest_map_root;
cmd_dest_map *cmd_dest_map_find;
/* data structs and vars for map implementation - end */

/**
 * Comparator function for map implementation
 */
int cmd_dest_map_compar(const void *l, const void *r) {
	const cmd_dest_map *lm = l;
	const cmd_dest_map *lr = r;
	return strcmp(lm->key, lr->key);
}

/**
 * Adds command-destination pair to map
 *
 * @param cmd pointer to command string
 * @param dest pointer to destination string
 */
void add_cmd_dest_pair(char *cmd, char *dest) {
	cmd_dest_map *pair= malloc(sizeof(cmd_dest_map));
	pair->key = cmd;
	pair->value = dest;
	tsearch(pair, &cmd_dest_map_root, cmd_dest_map_compar);
}

/**
 * Initializes command-destination map with command-destination pairs
 */
void init_cmd_dest_map(void) {
	cmd_dest_map_root = 0;
	cmd_dest_map_find = malloc(sizeof(cmd_dest_map));

	add_cmd_dest_pair("flag_recording", "track_manager");
	add_cmd_dest_pair("stop_recording", "recorder");
	//add_cmd_dest_pair("flag_recording_both_phrases", "track_manager");
	//add_cmd_dest_pair("cancel_recording", "track_manager");
	add_cmd_dest_pair("flag_switch_looping", "player");
	//add_cmd_dest_pair("mute_track", "player");
	add_cmd_dest_pair("flag_swap_versions", "track_manager");
	//add_cmd_dest_pair("flag_channel_switch_looping", "player");	
	//add_cmd_dest_pair("mute_channel", "player");

	add_cmd_dest_pair("set_tempo", "click");
	add_cmd_dest_pair("set_time_signature", "click");
}

/**
 * Retrieves a destination for given command
 *
 * @param s pointer to command string for which destination will be retrieved
 * @return pointer to destination string for given command
 */
char *get_dest_for_cmd(char *cmd) {
	cmd_dest_map_find->key = cmd;
	void *r = tfind(cmd_dest_map_find, &cmd_dest_map_root, cmd_dest_map_compar);
	if(r == NULL) {
		return NULL;
	} else {
		return (*(cmd_dest_map**)r)->value;
	}
}
