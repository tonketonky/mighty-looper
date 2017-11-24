#include "m_pd.h"  
#include "symb_id_map.h"
#include "helpers.h"
#include "stdlib.h"

#define PHRASE_1 gensym("p1")
#define PHRASE_2 gensym("p2")
#define VERSION_A gensym("a")
#define VERSION_B gensym("b")

// pointer to ml_track_manager class
static t_class *ml_track_manager_class;  
 
// data space for ml_track_manager class
typedef struct _ml_track_manager {  
  t_object  x_obj;  
  t_int 		layer_counters[2][2][4][2]; 	// 32 layer counters; 2 phrases x 2 channels x 4 tracks x 2 versions
	t_symbol 	*versions[2][2][4]; 					// 16 version holders; 2 phrases x 2 channels x 4 tracks
	t_int 		track_counters[2]; 						// 2 track counters; 2 phrases x 1 track count (regardless channel)

	t_int 		swap_versions_flags[2][4]; 		// 8 swap versions flags; 2 channels x 4 tracks

	t_symbol 	*current_phrase; 							// current phrase within which the looper operates

	// TODO: MACRO?
	t_symbol 	*channel_symbs[2]; 						// symbols of all possible channels
	t_symbol 	*track_symbs[4]; 							// symbols of all possible tracks

	t_atom 		cmd_args[7]; 									// array of arguments for commands that are sent out of the object

	t_outlet 	*cmd_out, *cmd_dest_out; 			// outlets for commands sent out of the object and for symbols that sets their destination
} t_ml_track_manager;  

/************************************************************
 * function declarations
 ************************************************************/

t_symbol *get_version_for_recording(t_ml_track_manager *x, t_symbol *phrase, t_symbol *channel, t_symbol *track);
t_int get_layer_num_for_recording(t_ml_track_manager *x, t_symbol *phrase, t_symbol *channel, t_symbol *track);

/************************************************************
 * initialization functions
 ************************************************************/

void reset_swap_versions_flags(t_ml_track_manager *x) {
	// initialize all swap versions flags to zeros
	for(int ch_id = 0; ch_id < 2; ch_id++) {
		for(int t_id = 0; t_id < 4; t_id++) {
			x->swap_versions_flags[ch_id][t_id] = 0;
		}
	}
}

void reset_layer_counters(t_ml_track_manager *x) {
	// initialize all layer counters to zeros 
	for(int p_id = 0; p_id < 2; p_id++) {
		for(int ch_id = 0; ch_id < 2; ch_id++) {
			for(int t_id = 0; t_id < 4; t_id++) {
				for(int v_id = 0; v_id <2; v_id++) {
					x->layer_counters[p_id][ch_id][t_id][v_id] = 0;
				}	
			}	
		}
	}
}

void reset_versions(t_ml_track_manager *x) {
	// initialize all version holders to symbol 'b' 
	for(int p_id=0; p_id<2; p_id++) {
		for(int ch_id=0; ch_id<2; ch_id++) {
			for(int t_id=0; t_id<4; t_id++) {
				x->versions[p_id][ch_id][t_id] = VERSION_B;
			}	
		}
	}
}

void reset_track_counters(t_ml_track_manager *x) {
	x->track_counters[0] = 0; // initialize track counter for phrase1 to zero
	x->track_counters[1] = 0; // initialize track counter for phrase2 to zero
}

/********************************************************************
 * data space member getters
 ********************************************************************/

t_int *get_layer_counter(t_ml_track_manager *x, t_symbol *phrase, t_symbol *channel, t_symbol *track, t_symbol *version) {
	int phrase_id = get_id_for_symb(phrase);
	int channel_id = get_id_for_symb(channel);
	int track_id = get_id_for_symb(track);
	int version_id = get_id_for_symb(version);

	return &(x->layer_counters[phrase_id][channel_id][track_id][version_id]);
}

/********************************************************************
 * data space values getters
 ********************************************************************/

t_symbol *get_version(t_ml_track_manager *x, t_symbol *phrase, t_symbol *channel, t_symbol *track) {
	int phrase_id = get_id_for_symb(phrase);
	int channel_id = get_id_for_symb(channel);
	int track_id = get_id_for_symb(track);

	return x->versions[phrase_id][channel_id][track_id];
}

/********************************************************************
 * output commands functions
 ********************************************************************/

t_symbol *get_tabplay_dest(t_symbol *channel, t_symbol *track, t_int layer) {
	char *channel_str = malloc(5);
	symb_2_string(channel, channel_str);
	char *track_str = malloc(5);
	symb_2_string(track, track_str);

	char *tabplay_dest_str = malloc(25);

	sprintf(
		tabplay_dest_str,
		"tabplay_%s_%s_l%lu",
		channel_str,
		track_str,
		layer
	);

	free(channel_str);
	free(track_str);

	t_symbol *tabplay_dest = gensym(tabplay_dest_str);
	free(tabplay_dest_str);

	return tabplay_dest; 
}

void tabplay_set_table(t_ml_track_manager *x, t_symbol *phrase, t_symbol *channel, t_symbol *track, t_int layer, t_symbol *version) {

	t_symbol *tabplay_dest = get_tabplay_dest(channel, track, layer);

	outlet_symbol(x->cmd_dest_out, tabplay_dest);

	SETSYMBOL(
		x->cmd_args,
		get_table_name_symb(
			phrase, 
			channel, 
			track,
			layer,
			version
		)
	);

	outlet_anything(x->cmd_out, gensym("stop"), 0, 0);
	outlet_anything(x->cmd_out, gensym("set"), 1, x->cmd_args);
}

void tabplay_set_nothing(t_ml_track_manager *x, t_symbol *channel, t_symbol *track, t_int layer) {

	t_symbol *tabplay_dest = get_tabplay_dest(channel, track, layer);

	outlet_symbol(x->cmd_dest_out, tabplay_dest);

	outlet_anything(x->cmd_out, gensym("set"), 0, 0);
}

void player_set_looping(t_ml_track_manager *x, t_symbol *channel, t_symbol *track, t_int is_looping) {
	outlet_symbol(x->cmd_dest_out, gensym("player"));

	SETSYMBOL(x->cmd_args, channel);
	SETSYMBOL(x->cmd_args+1, track);
	SETFLOAT(x->cmd_args+2, is_looping);
	outlet_anything(x->cmd_out, gensym("set_looping"), 3, x->cmd_args);
}

void recorder_flag_recording(t_ml_track_manager *x, t_symbol *phrase, t_symbol *channel, t_symbol *track, t_symbol *alloc_type, t_int countdown) {
	outlet_symbol(x->cmd_dest_out, gensym("recorder"));

	SETSYMBOL(x->cmd_args, phrase);
	SETSYMBOL(x->cmd_args+1, channel);
	SETSYMBOL(x->cmd_args+2, track);
	SETFLOAT(x->cmd_args+3, get_layer_num_for_recording(x, phrase, channel, track));
	SETSYMBOL(x->cmd_args+4, get_version_for_recording(x, phrase, channel, track));	
	SETSYMBOL(x->cmd_args+5, alloc_type);
	SETFLOAT(x->cmd_args+6, countdown);

	outlet_anything(x->cmd_out, gensym("flag_recording"), 7, x->cmd_args);
}

void layer_merger_flag_merging(t_ml_track_manager *x, t_symbol *phrase, t_symbol *channel, t_symbol *track, t_int num_of_layers, t_symbol *alloc_type) {
	outlet_symbol(x->cmd_dest_out, gensym("layer_merger"));

	SETSYMBOL(x->cmd_args, phrase);
	SETSYMBOL(x->cmd_args+1, channel);
	SETSYMBOL(x->cmd_args+2, track);
	SETSYMBOL(x->cmd_args+3, get_version_for_recording(x, phrase, channel, track));	
	SETFLOAT(x->cmd_args+4, num_of_layers);
	SETSYMBOL(x->cmd_args+5, alloc_type);

	outlet_anything(x->cmd_out, gensym("flag_merging"), 6, x->cmd_args);
}

/********************************************************************
 * layers and versions management functions
 ********************************************************************/

t_symbol *get_last_version(t_ml_track_manager *x, t_symbol *phrase, t_symbol *channel, t_symbol *track) {
	int ch_id = get_id_for_symb(channel);
	int t_id = get_id_for_symb(track);
	
	t_symbol *last_version;

	if(x->swap_versions_flags[ch_id][t_id] == 0) {
		last_version = get_version(x, phrase, channel, track);
	} else {
		last_version = get_opp_version(get_version(x, phrase, channel, track));
	}	
	return last_version;
}

t_symbol *get_version_for_recording(t_ml_track_manager *x, t_symbol *phrase, t_symbol *channel, t_symbol *track) {
	return get_opp_version(get_last_version(x, phrase, channel, track));
}

t_int get_layer_num_for_recording(t_ml_track_manager *x, t_symbol *phrase, t_symbol *channel, t_symbol *track) {

	t_int *layer_counter = get_layer_counter(x, phrase, channel, track, get_last_version(x, phrase, channel, track));

	t_int layer_num_for_recording;

	if(*layer_counter < 2) {
		// there is only 0 or 1 layer, next layer number is returned
		layer_num_for_recording = *layer_counter + 1;
	} else {
		// there already are 2 layers, layer number 2 is returned in order to replace the last layer
		layer_num_for_recording = 2;
	}

	return layer_num_for_recording;
}

void swap_versions(t_ml_track_manager *x, t_symbol *phrase, t_symbol *channel, t_symbol *track) {
	t_int p_id = get_id_for_symb(phrase);
	t_int ch_id = get_id_for_symb(channel);		
	t_int t_id = get_id_for_symb(track);

	t_symbol **version = &x->versions[p_id][ch_id][t_id];

	*version = *version == VERSION_A ? VERSION_B : VERSION_A;

	t_int *curr_ver_layer_counter = get_layer_counter(x, phrase, channel, track, *version);
	t_int *prev_ver_layer_counter = get_layer_counter(x, phrase, channel, track, get_opp_version(*version));

	if(*prev_ver_layer_counter == 0 && *curr_ver_layer_counter == 1) { 
		// first layer added -> new track added -> increment track count for given phrase	
		x->track_counters[p_id]++;
		// start looping
		player_set_looping(x, channel, track, 1);
	} else if (*prev_ver_layer_counter == 1 && *curr_ver_layer_counter == 0) {
		// the only layer removed -> existing track removed -> decrement track count for given phrase
		x->track_counters[p_id]--;
		// stop looping
		player_set_looping(x, channel, track, 0);
	}

	// set table to tabplay for all existing layers
	for(int layer = 1; layer <= *curr_ver_layer_counter && layer <= 2; layer++ ) {
		tabplay_set_table(x, phrase, channel, track, layer, *version);
	}

	// set nothing to tabplay for all non-existing layers
	for(int layer = 2; layer > *curr_ver_layer_counter && layer > 0; layer-- ) {
		tabplay_set_nothing(x, channel, track, layer);
	}
}

void flag_unflag_swap_versions(t_ml_track_manager *x, t_symbol *channel, t_symbol *track) {
	int ch_id = get_id_for_symb(channel);
	int t_id = get_id_for_symb(track);
	x->swap_versions_flags[ch_id][t_id] = x->swap_versions_flags[ch_id][t_id] ^ 1;	
}

/*******************************************************************************
 * ml_track_manager class methods
 *******************************************************************************/

void ml_track_manager_flag_recording(t_ml_track_manager *x, t_symbol *channel, t_symbol *track) {
	int p_id = get_id_for_symb(x->current_phrase);
	
	t_symbol *rec_alloc_method;

	t_symbol *last_version = get_last_version(x, x->current_phrase, channel, track);

	t_int last_ver_layer_count = *get_layer_counter(x, x->current_phrase, channel, track, last_version);
	t_int next_ver_layer_count = *get_layer_counter(x, x->current_phrase, channel, track, get_opp_version(last_version));

	if(x->track_counters[p_id] == 0) {
		// 1st track of the phrase -> allocate dynamically
		rec_alloc_method = gensym("dynamic");
	} else {
		// not 1st track of the phrase 

		/* NOTE:
		 * first 3 layers are recorded each to new table -> allocation is needed unless there already is undone version
		 * meaning that the table has already been allocated while undone version was recorded -> no need for allocation
		 * (undone version has always more layers than current version)
		 */
		if(last_ver_layer_count < 3 && last_ver_layer_count >= next_ver_layer_count) {
			// currently there are less than 3 layers recorded and undone version does not exist -> need for allocation
			rec_alloc_method = gensym("static");
		} else {
			// first 3 layers are already recorded or undone version exists -> next layer will be recorded to already allocated table -> no need for allocation
			rec_alloc_method = gensym("none");
		}
	}

	t_int countdown;

	if(x->track_counters[0] == 0 && x->track_counters[1] == 0) {
		countdown = 2;
	} else {
		countdown = -1;
	}

	recorder_flag_recording(x, x->current_phrase, channel, track, rec_alloc_method, countdown);

	/* NOTE:
	 * only 1st marged layer is written to new table -> allocation is needed unless there already is undone version
	 * meaning that the table has already been allocated while undone version was recorded -> no need for allocation
	 * (undone version has always more layers than current version)
	 */

	if(last_ver_layer_count != 0) {
		// there already is(are) layer(s) -> need for merging

		t_int num_of_layers_to_merge;
		t_symbol *merge_alloc_method;

		if(last_ver_layer_count == 1) {
			// there already is 1 layer -> merge 1 layer
				num_of_layers_to_merge = 1;

				if(last_ver_layer_count > next_ver_layer_count) {
					// undone version does not exist -> need for allocation
					merge_alloc_method = gensym("static");
				} else {
					// undone version exists -> no need for allocation
					merge_alloc_method = gensym("none");
				}
			} else {
			// there already are 2 or more layers -> merge last 2 layers; no need for allocation
			num_of_layers_to_merge = 2;
			merge_alloc_method = gensym("none");
		}
		layer_merger_flag_merging(x, x->current_phrase, channel, track, num_of_layers_to_merge, merge_alloc_method);
	}
}

void ml_track_manager_recording_started(t_ml_track_manager *x, t_symbol *channel, t_symbol *track) {

	// this happens before swapping versions after recording therefore last version is still current one and new version is opposite to it
	t_symbol *last_version = get_version(x, x->current_phrase, channel, track);
	t_symbol *new_version = get_opp_version(last_version);

	t_int *new_ver_layer_counter = get_layer_counter(x, x->current_phrase, channel, track, new_version);
	t_int *last_ver_layer_counter = get_layer_counter(x, x->current_phrase, channel, track, last_version);

	// new version layer count is last version layer count incremented by 1
	*new_ver_layer_counter = *last_ver_layer_counter + 1;

	// flag swapping versions on new cycle
	flag_unflag_swap_versions(x, channel, track);
}

void ml_track_manager_swap_phrases(t_ml_track_manager *x) {
	x->current_phrase = x->current_phrase == PHRASE_1 ? PHRASE_2 : PHRASE_1;
}

void ml_track_manager_flag_swap_versions(t_ml_track_manager *x, t_symbol *channel, t_symbol *track) {
	flag_unflag_swap_versions(x, channel, track);
}

void ml_track_manager_set_up_new_cycle(t_ml_track_manager *x) {
	for(int ch_id = 0; ch_id < 2; ch_id++) {
		for(int t_id = 0; t_id < 4; t_id++) {
			if(x->swap_versions_flags[ch_id][t_id] == 1) {
				x->swap_versions_flags[ch_id][t_id] = x->swap_versions_flags[ch_id][t_id] ^ 1;
				swap_versions(x, x->current_phrase, x->channel_symbs[ch_id], x->track_symbs[t_id]);
			}
		}
	}
}

/**************************************************************
 * constructor
 **************************************************************/

void *ml_track_manager_new(void) {  
  t_ml_track_manager *x = (t_ml_track_manager *)pd_new(ml_track_manager_class);  
	
	reset_layer_counters(x);
	reset_versions(x);
	reset_track_counters(x);
	reset_swap_versions_flags(x);

	init_symb_id_map();

	x->channel_symbs[0] = gensym("ch1");
	x->channel_symbs[1] = gensym("ch2");
	x->track_symbs[0] = gensym("t1");
	x->track_symbs[1] = gensym("t2");
	x->track_symbs[2] = gensym("t3");
	x->track_symbs[3] = gensym("t4");

	x->current_phrase = PHRASE_1;

	// outlets
	x->cmd_out = outlet_new(&x->x_obj, 0);
	x->cmd_dest_out = outlet_new(&x->x_obj, &s_symbol);
	return (void *)x;  
}  

/**************************************************************
 * destructor
 **************************************************************/

void ml_track_manager_free(t_ml_track_manager *x) {
	outlet_free(x->cmd_out);
	outlet_free(x->cmd_dest_out);
}

/**************************************************************
 * Setup function for ml_track_manager class
 **************************************************************/

void ml_track_manager_setup(void) {  
  ml_track_manager_class = class_new(gensym("ml_track_manager"),  
        (t_newmethod)ml_track_manager_new,  
        (t_method)ml_track_manager_free, 
				sizeof(t_ml_track_manager),  
        CLASS_DEFAULT, 0);  

  class_addmethod(ml_track_manager_class, 
		(t_method)ml_track_manager_flag_recording,
		gensym("flag_recording"),
		A_DEFSYMBOL,
		A_DEFSYMBOL, 0);

	class_addmethod(ml_track_manager_class,
		(t_method)ml_track_manager_recording_started,
		gensym("recording_started"),
		A_DEFSYMBOL,
		A_DEFSYMBOL, 0);

	class_addmethod(ml_track_manager_class, 
		(t_method)ml_track_manager_swap_phrases,
		gensym("swap_phrases"), 0);

  class_addmethod(ml_track_manager_class, 
		(t_method)ml_track_manager_flag_swap_versions,
		gensym("flag_swap_versions"),
		A_DEFSYMBOL, 
		A_DEFSYMBOL, 0);
 
	class_addmethod(ml_track_manager_class, 
		(t_method)ml_track_manager_set_up_new_cycle,
		gensym("set_up_new_cycle"), 0);
}
