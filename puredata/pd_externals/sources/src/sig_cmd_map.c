#include "m_pd.h"  
#include "search.h"
#include "stdlib.h"
#include "string.h"
#include "stdio.h"

/* data structs and vars for map implementation - begin */
typedef struct {
	char *key;
	char *value;
} sig_cmd_map;

void *sig_cmd_map_root;
sig_cmd_map *sig_cmd_map_find;
/* data structs and vars for map implementation - end */

/**
 * Comparator function for map implementation
 */
int sig_cmd_map_compar(const void *l, const void *r) {
	const sig_cmd_map *lm = l;
	const sig_cmd_map *lr = r;
	return strcmp(lm->key, lr->key);
}
	
/**
 * Adds signal-command pair to map
 *
 * @param sig pointer to signal string
 * @param cmd pointer to command string 
 */
void add_sig_cmd_pair(char *sig, char *cmd) {
	sig_cmd_map *pair= malloc(sizeof(sig_cmd_map));
	pair->key = sig;
	pair->value = cmd;
	tsearch(pair, &sig_cmd_map_root, sig_cmd_map_compar);
}

/**
 * Initializes signal-command map with signal-command pairs for record and play buttons
 */
void init_sig_cmd_map(void) {
	sig_cmd_map_root = 0;
	sig_cmd_map_find = malloc(sizeof(sig_cmd_map));

	add_sig_cmd_pair("r_sp_t_r=0", "flag_recording");
	add_sig_cmd_pair("r_sp_t_r=1", "stop_recording");
	//add_sig_cmd_pair("r_lp_t_r=0", "flag_recording_both_phrases");
	//add_sig_cmd_pair("r_lp_t_r=1", "cancel_recording");
	add_sig_cmd_pair("p_sp_t", "flag_switch_looping");
	//add_sig_cmd_pair("p_dp_t", "mute_track");
	add_sig_cmd_pair("p_lp_t", "flag_swap_versions");
	//add_sig_cmd_pair("p_sp_ch", "flag_channel_switch_looping");	
	//add_sig_cmd_pair("p_dp_ch", "mute_channel");
}

char *get_map_value(char *key) {
	sig_cmd_map_find->key = key;
	void *r = tfind(sig_cmd_map_find, &sig_cmd_map_root, sig_cmd_map_compar);
	if(r == NULL) {
		return NULL;
	} else {
		return (*(sig_cmd_map**)r)->value;
	}
}

/**
 * Retrieves a command for play signal
 *
 * @param s pointer to signal string for which command will be retrieved
 * @return pointer to command string for given signal
 */
char *get_cmd_for_play_sig(char *sig) {
	return get_map_value(sig);
}

/**
 * Retrieves a command for record signal based on value in is_recording argument
 *
 * @param s pointer to signal string for which command will be retrieved
 * @param is_recording specifies which recording specific command shall be retrieved
 * @return pointer to command string for given signal
 */
char *get_cmd_for_rec_sig(char *sig, int is_recording) {
	char *rec_specific_sig = malloc(sizeof(sig) + 4);
	sprintf(rec_specific_sig, "%s_r=%d", sig, is_recording);
	char *ret_cmd = get_map_value(rec_specific_sig);
	free(rec_specific_sig);
	return ret_cmd;
}
