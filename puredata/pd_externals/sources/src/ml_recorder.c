#include "m_pd.h"  
#include "symb_id_map.h"
#include "helpers.h"
#include "stdlib.h"

#define PHRASE1 gensym("p1")
#define PHRASE2 gensym("p2")

// pointer to ml_recorder class
static t_class *ml_recorder_class;  
 
// data space for ml_recorder class
typedef struct _ml_recorder {  
  t_object  x_obj;  

	t_int 		is_flagged; 							// indicates whether a track is flagged for recording

	t_symbol 	*flagged_phrase; 					// phrase flagged for recording
	t_symbol 	*flagged_channel; 				// channel flagged for recording
	t_symbol 	*flagged_track; 					// track flagged for recording
	t_int 		flagged_layer; 						// layer flagged for recording
	t_symbol 	*flagged_version; 				// version flagged for recording

	t_symbol 	*flagged_alloc_method; 		// allocation method for table flagged for recording

	t_int 		is_recording; 						// indicates whether a track is being recorded

	t_symbol 	*recorded_phrase; 				// currently recorded track's corresponding phrase
	t_symbol 	*recorded_channel; 				// currently recorded track's corresponding channel
	t_symbol 	*recorded_track; 					// currently recorded track

	t_symbol 	*recorded_alloc_method; 	// allocation method for currently recorded table

	t_int 		rec_countdown; 						// countdown for recording start, used when initial track of whole looper is about to be recorded

	t_atom 		cmd_args[5]; 							// array of arguments for commands that are sent out of the object

	t_outlet 	*cmd_out, *cmd_dest_out; 	// outlets for commands sent out of the object and for symbols that sets their destination
} t_ml_recorder;  


/******************************************************************
 * track recording management functions
 ******************************************************************/

void flag_track(t_ml_recorder *x, t_symbol *phrase, t_symbol *channel, t_symbol *track, t_int layer, t_symbol *version, t_symbol *alloc_method, t_int countdown) {
	x->is_flagged = 1;
	x->flagged_alloc_method = alloc_method;
	
	x->rec_countdown = countdown;

	x->flagged_phrase = phrase;
	x->flagged_channel = channel;
	x->flagged_track = track;
	x->flagged_layer = layer;
	x->flagged_version = version;
}

/******************************************************************
 * output commands functions
 ******************************************************************/

void notify_track_manager(t_ml_recorder *x) {
	outlet_symbol(x->cmd_dest_out, gensym("track_manager"));
	SETSYMBOL(x->cmd_args, x->recorded_phrase);
	SETSYMBOL(x->cmd_args+1, x->recorded_channel);
	SETSYMBOL(x->cmd_args+2, x->recorded_track);
	outlet_anything(x->cmd_out, gensym("recording_stopped"), 3, x->cmd_args);
}

void start_recording(t_ml_recorder *x) {
	if(x->flagged_alloc_method != gensym("none")) {
		// allocation method is not "none" -> allocation will be needed
		outlet_symbol(x->cmd_dest_out, gensym("table_allocator"));
		
		SETSYMBOL(x->cmd_args, x->flagged_phrase);
		SETSYMBOL(x->cmd_args+1, x->flagged_channel);
		SETSYMBOL(x->cmd_args+2, x->flagged_track);
		SETFLOAT(x->cmd_args+3, x->flagged_layer);
		SETSYMBOL(x->cmd_args+4, x->flagged_version);
	
		t_symbol* cmd = NULL;

		if(x->flagged_alloc_method == gensym("dynamic")) {
			// allocation method is "dynamic" -> start dynamic allocation
			cmd = gensym("start_dynamic_allocation");	
		} else if(x->flagged_alloc_method == gensym("static")) {
			// allocation method is "static" -> allocate
			cmd = gensym("allocate");
		}

		outlet_anything(x->cmd_out, cmd, 5, x->cmd_args);
	}

	outlet_symbol(x->cmd_dest_out, gensym("tabwrite_rec"));

	SETSYMBOL(
		x->cmd_args,
		get_table_name_symb(
			x->flagged_phrase, 
			x->flagged_channel, 
			x->flagged_track,
			x->flagged_layer,
			x->flagged_version
		)
	);
	
	outlet_anything(x->cmd_out, gensym("set"), 1, x->cmd_args);
	outlet_bang(x->cmd_out);

	x->is_flagged = 0;
	x->is_recording = 1;
	
	x->recorded_phrase = x->flagged_phrase;
	x->recorded_channel = x->flagged_channel;
	x->recorded_track = x->flagged_track;

	x->recorded_alloc_method = x->flagged_alloc_method;
}

/*******************************************************************************
 * ml_recorder class methods
 *******************************************************************************/

void ml_recorder_flag_recording(t_ml_recorder *x, t_symbol *s, int argc, t_atom *argv) {	

	t_symbol *phrase = atom_getsymbol(argv);
	t_symbol *channel = atom_getsymbol(argv+1);
	t_symbol *track = atom_getsymbol(argv+2);
	t_int layer = atom_getint(argv+3);
	t_symbol *version = atom_getsymbol(argv+4);
	t_symbol *alloc_method = atom_getsymbol(argv+5);
	t_int countdown = atom_getint(argv+6);

	if(x->is_flagged == 0) {
		flag_track(x, phrase, channel, track, layer, version, alloc_method, countdown);
	} else{
		if(x->flagged_phrase == phrase && x->flagged_channel == channel && x->flagged_track == track) {
			x->is_flagged = 0;
			x->rec_countdown = -1;
		} else {
			flag_track(x, phrase, channel, track, layer, version, alloc_method, countdown);
		}
	}
}

void ml_recorder_stop_recording(t_ml_recorder *x, t_symbol *channel, t_symbol *track) {
	if(x->is_recording == 1 && channel == x->recorded_channel && track == x->recorded_track) {
		x->is_recording = 0;
		if(x->recorded_alloc_method == gensym("dynamic")) {
			outlet_symbol(x->cmd_dest_out, gensym("table_allocator"));
			outlet_anything(x->cmd_out, gensym("stop_dynamic_allocation"), 0, 0);
		}	else {
			outlet_symbol(x->cmd_dest_out, gensym("tabwrite_rec"));
			outlet_symbol(x->cmd_out, gensym("stop"));
		}
		notify_track_manager(x);		
	}
}

void ml_recorder_set_up_new_cycle(t_ml_recorder *x) {
	if(x->is_recording == 1) {
		x->is_recording = 0;
		notify_track_manager(x);
	}	
}

void ml_recorder_new_cycle(t_ml_recorder *x) {
	if(x->is_flagged == 1) {
		start_recording(x);
	}	
}

void ml_recorder_new_bar(t_ml_recorder *x) {
	if(x->rec_countdown != -1) {
		// countdown for recording is set
		char *count_str = malloc(12);

		// print countdown to console
		sprintf(count_str, "%lu", x->rec_countdown);
		post(count_str);
		free(count_str);
		
		if(x->rec_countdown-- == 0) {
			// if countdown is 0 start recording; decrement countdown to -1
			start_recording(x);
		}
	}
}

/**************************************************************
 * constructor
 **************************************************************/

void *ml_recorder_new(void) {  
  t_ml_recorder *x = (t_ml_recorder *)pd_new(ml_recorder_class);  
	
	init_symb_id_map();
	
	x->is_recording = 0;
	x->is_flagged = 0;

	x->rec_countdown = -1;
	
	// outlets
	x->cmd_out = outlet_new(&x->x_obj, 0);
	x->cmd_dest_out = outlet_new(&x->x_obj, &s_symbol);
	
	return (void *)x;  
}  

/**************************************************************
 * destructor
 **************************************************************/

void ml_recorder_free(t_ml_recorder *x) {
	outlet_free(x->cmd_out);
	outlet_free(x->cmd_dest_out);
}

/**************************************************************
 * Setup function for ml_recorder class
 **************************************************************/

void ml_recorder_setup(void) {  
  ml_recorder_class = class_new(gensym("ml_recorder"),  
        (t_newmethod)ml_recorder_new,  
        (t_method)ml_recorder_free,
				sizeof(t_ml_recorder),  
        CLASS_DEFAULT, 0);  

  class_addmethod(ml_recorder_class, 
		(t_method)ml_recorder_flag_recording,
		gensym("flag_recording"),
		A_GIMME, 0);

	class_addmethod(ml_recorder_class,
		(t_method)ml_recorder_stop_recording,
		gensym("stop_recording"),
		A_DEFSYMBOL,
		A_DEFSYMBOL, 0);

	class_addmethod(ml_recorder_class, 
		(t_method)ml_recorder_set_up_new_cycle,
		gensym("set_up_new_cycle"), 0);

	class_addmethod(ml_recorder_class, 
		(t_method)ml_recorder_new_cycle,
		gensym("new_cycle"), 0);

	class_addmethod(ml_recorder_class, 
		(t_method)ml_recorder_new_bar,
		gensym("new_bar"), 0);
}
