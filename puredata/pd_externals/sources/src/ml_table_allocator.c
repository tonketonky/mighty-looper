#include "m_pd.h"  
#include "symb_id_map.h"
#include "helpers.h"

#define PHRASE1 gensym("p1")
#define PHRASE2 gensym("p2")

// pointer to ml_table_allocator class
static t_class *ml_table_allocator_class;  
 
// data space for ml_table_allocator class
typedef struct _ml_table_allocator {  
  t_object  x_obj;  
	t_int 		tracks_sizes[2]; 					// 2 size holders for ALL tracks per phrase; 2 phrases x 1 tracks size
	
	t_int 		tempo; 										// tempo in bpm
	t_int 		beat_note_lengths[2]; 		// lengths of the beat notes (time signature denominator); 2 phrases x 1 beat note length;  whole=1, half=2, quarter=4
														
	t_int 		beat_sizes[2];						// number of samples per 1 beat; 2 phrases x 1 beat size
	t_int 		sample_rate; 							// samples per second

	t_int 	 	beat_counts[2]; 					// number of beats per cycle for phrase; 2 phrases x 1 beat count

	t_int 		is_dynamic_allocation; 		// indicated whether currently recorded track is dynamically reallocated  

	t_symbol 	*dyn_alloc_phrase; 				// corresponding phrase for dynamically allocated table
	t_symbol 	*dyn_alloc_channel; 			// corresponding channel for dynamically allocated table
	t_symbol 	*dyn_alloc_track; 				// corresponding track for dynamically allocated table
	t_int 	 	dyn_alloc_layer; 					// corresponding layer for dynamically allocated table
	t_symbol 	*dyn_alloc_version; 			// corresponding version for dynamically allocated table

	t_atom 		cmd_args[3]; 							// array of arguments for commands that are sent out of the object

	t_outlet 	*cmd_out, *cmd_dest_out; 	// outlets for commands sent out of the object and for symbols that sets their destination
} t_ml_table_allocator;  

/********************************************************************
 * data space values update functions
 ********************************************************************/

void recalc_beat_size(t_ml_table_allocator *x, t_symbol *phrase) {
	t_int p_id = get_id_for_symb(phrase);
	/* NOTE:
	 * beat size (in number of samples) is calculated as follows:
	 * 60 (seconds) / tempo (in bpm - quarter note beats per minute) = 1 quarter note beat duration (in seconds) (tempo is in quarter note beats per second)
	 * 4 (because tempo is in quarter note beats per second) / beat note length (time signature denominator) = coefficient for beat duration according to time signature
	 * (e.g. time signature is 4/4 => 1 beat with 60bpm tempo takes 1 second; time signature is 4/8 => 1 beat with 60 bpm tempo takes 0.5 second)
	 * 1 quarter note beat duration (in seconds) * coefficient for beat duration = beat duration according to time signature
	 * beat duration according to time signature * sample rate (in samples per second) = number of samples per 1 beat according to time signature
	 */
	x->beat_sizes[p_id] = 60.0 / x->tempo * 4 / x->beat_note_lengths[p_id] * x->sample_rate;	
}

/***********************************************************************
 * output commands functions
 ***********************************************************************/

void resize_track(t_ml_table_allocator *x, t_symbol *phrase, t_symbol *channel, t_symbol *track, t_int layer, t_symbol *version, t_int new_size) {
	outlet_symbol(x->cmd_dest_out, gensym("resize"));
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
	SETFLOAT(x->cmd_args+1, new_size);
	outlet_list(x->cmd_out, &s_list, 2, x->cmd_args);
}

void click_set_beat_count(t_ml_table_allocator *x, t_int beat_count) {
	outlet_symbol(x->cmd_dest_out, gensym("click"));
	SETFLOAT(x->cmd_args, beat_count);
	outlet_anything(x->cmd_out, gensym("set_beat_count"), 1, x->cmd_args);
}

/*******************************************************************************
 * ml_table_allocator class methods
 *******************************************************************************/

void ml_table_allocator_start_dynamic_allocation(t_ml_table_allocator *x, t_symbol *phrase, t_symbol *channel, t_symbol *track, t_floatarg layer, t_symbol *version) {
	x->is_dynamic_allocation = 1;

	t_int p_id = get_id_for_symb(phrase);
	t_int *tracks_size = &x->tracks_sizes[p_id];

	*tracks_size = x->beat_sizes[p_id];

	resize_track(x, phrase, channel, track, layer, version, *tracks_size);

	x->dyn_alloc_phrase = phrase;
	x->dyn_alloc_channel = channel;
	x->dyn_alloc_track = track;
	x->dyn_alloc_layer = layer;
	x->dyn_alloc_version = version;
}

void ml_table_allocator_stop_dynamic_allocation(t_ml_table_allocator *x) {
	x->is_dynamic_allocation = 0;
			
	t_int p_id = get_id_for_symb(x->dyn_alloc_phrase);
	x->tracks_sizes[p_id] -= x->beat_sizes[p_id];
	resize_track(x, x->dyn_alloc_phrase, x->dyn_alloc_channel, x->dyn_alloc_track, x->dyn_alloc_layer, x->dyn_alloc_version, x->tracks_sizes[p_id]);
	click_set_beat_count(x, x->beat_counts[p_id]);
}

void ml_table_allocator_allocate_track(t_ml_table_allocator *x, t_symbol *phrase, t_symbol *channel, t_symbol *track, t_floatarg layer, t_symbol *version) {
	t_int p_id = get_id_for_symb(phrase);
	t_int *tracks_size = &x->tracks_sizes[p_id];

	resize_track(x, phrase, channel, track, layer, version, *tracks_size);
}

void ml_table_allocator_set_tempo(t_ml_table_allocator *x, t_floatarg tempo) {
	x->tempo = tempo;
	recalc_beat_size(x, PHRASE1);
	recalc_beat_size(x, PHRASE2);
}

void ml_table_allocator_set_beat_note_length(t_ml_table_allocator *x, t_symbol *phrase, t_floatarg beat_note_length) {
	t_int p_id = get_id_for_symb(phrase);
	x->beat_note_lengths[p_id] = beat_note_length;
	recalc_beat_size(x, phrase);
}

void ml_table_allocator_new_beat(t_ml_table_allocator *x) {
	if(x->is_dynamic_allocation == 1) {
		t_int p_id = get_id_for_symb(x->dyn_alloc_phrase);
		x->tracks_sizes[p_id] += x->beat_sizes[p_id];
		resize_track(x, x->dyn_alloc_phrase, x->dyn_alloc_channel, x->dyn_alloc_track, x->dyn_alloc_layer, x->dyn_alloc_version, x->tracks_sizes[p_id]);
		x->beat_counts[p_id]++;	
	}
}

/**************************************************************
 * constructor
 **************************************************************/

void *ml_table_allocator_new(void) {  
  t_ml_table_allocator *x = (t_ml_table_allocator *)pd_new(ml_table_allocator_class);  
	
	init_symb_id_map();
	
	x->sample_rate = 44100;

	x->tempo = 60;

	ml_table_allocator_set_beat_note_length(x, PHRASE1, 4);
	ml_table_allocator_set_beat_note_length(x, PHRASE2, 4);

	x->tracks_sizes[0] = 0;
	x->tracks_sizes[1] = 0;

	x->beat_counts[0] = 0;
	x->beat_counts[1] = 0;

	x->is_dynamic_allocation = 0;

	// outlets
	x->cmd_out = outlet_new(&x->x_obj, 0);
	x->cmd_dest_out = outlet_new(&x->x_obj, &s_symbol);
	return (void *)x;  
}  

/**************************************************************
 * destructor
 **************************************************************/

void ml_table_allocator_free(t_ml_table_allocator *x) {
	outlet_free(x->cmd_out);
	outlet_free(x->cmd_dest_out);
}

/**************************************************************
 * Setup function for ml_table_allocator class
 **************************************************************/

void ml_table_allocator_setup(void) {  
  ml_table_allocator_class = class_new(gensym("ml_table_allocator"),  
        (t_newmethod)ml_table_allocator_new,  
        (t_method)ml_table_allocator_free, 
				sizeof(t_ml_table_allocator),  
        CLASS_DEFAULT, 0);  

  class_addmethod(ml_table_allocator_class, 
		(t_method)ml_table_allocator_start_dynamic_allocation,
		gensym("start_dynamic_allocation"),
		A_DEFSYMBOL,
		A_DEFSYMBOL,
		A_DEFSYMBOL,
		A_DEFFLOAT,
		A_DEFSYMBOL, 0);

	class_addmethod(ml_table_allocator_class,
		(t_method)ml_table_allocator_stop_dynamic_allocation,
		gensym("stop_dynamic_allocation"), 0);

	 class_addmethod(ml_table_allocator_class, 
		(t_method)ml_table_allocator_allocate_track,
		gensym("allocate"),
		A_DEFSYMBOL,
		A_DEFSYMBOL,
		A_DEFSYMBOL,
		A_DEFFLOAT,
		A_DEFSYMBOL, 0);

	class_addmethod(ml_table_allocator_class, 
		(t_method)ml_table_allocator_set_tempo,
		gensym("set_tempo"),
		A_DEFFLOAT, 0);

  class_addmethod(ml_table_allocator_class, 
		(t_method)ml_table_allocator_set_beat_note_length,
		gensym("set_beat_note"),
		A_DEFSYMBOL,
		A_DEFFLOAT, 0);

	class_addmethod(ml_table_allocator_class,
		(t_method)ml_table_allocator_new_beat,
		gensym("new_beat"), 0);
}
