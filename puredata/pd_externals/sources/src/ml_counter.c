#include "m_pd.h"  
#include "search.h"
#include "stdlib.h"
#include "string.h"
#include "stdio.h"
#include "symb_id_map.h"

// pointer to ml_counter class
static t_class *ml_counter_class;  
 
// data space for ml_counter class
typedef struct _ml_counter {  
  t_object  x_obj;  
  t_int 		counters[2][2][4]; 		// 16 counters; 2 phrases x 2 channels x 4 tracks
} t_ml_counter;  

/**
 * Sends value to ml_counter outlet
 *
 * @param x pointer to ml_counter object
 * @param value value to be sent to outlet
 */
void output_value(t_ml_counter *x, int value) {
	outlet_float(x->x_obj.ob_outlet, value);
}


/**
 * Retrieves desired counter from the array of counters for given phrase, channel and track
 *
 * @param x pointer to ml_counter object
 * @param phrase pointer to t_symbol of phrase
 * @param channel pointer to t_symbol of phrase
 * @param track pointer to t_symbol of track
 * @return pointer to counter for given phrase, channel and track
 */
t_int *get_counter(t_ml_counter *x, t_symbol *phrase, t_symbol *channel, t_symbol *track) {
	int phrase_id = get_id_for_symb(phrase);
	int channel_id = get_id_for_symb(channel);
	int track_id = get_id_for_symb(track);

	return &(x->counters[phrase_id][channel_id][track_id]);
}

/**
 * Sends current counter value for given phrase, channel and track to outlet
 *
 * @param x pointer to ml_counter object
 * @param phrase pointer to t_symbol of phrase
 * @param channel pointer to t_symbol of channel
 * @param track pointer to t_symbol of track
 */
void get(t_ml_counter *x, t_symbol *phrase, t_symbol *channel, t_symbol *track) { 
	output_value(x, *get_counter(x, phrase, channel, track));	
}  

/**
 * Increments counter value for given phrase, channel and track and sends it to outlet 
 *
 * @param x pointer to ml_counter object
 * @param phrase pointer to t_symbol of phrase
 * @param channel pointer to t_symbol of channel
 * @param track pointer to t_symbol of track
 */
void increment(t_ml_counter *x, t_symbol *phrase, t_symbol *channel, t_symbol *track) {  
	output_value(x, ++*get_counter(x, phrase, channel, track));
} 

/**
 * Decrements counter value for given phrase, channel and track and sends it to outlet 
 *
 * @param x pointer to ml_counter object
 * @param phrase pointer to t_symbol of phrase
 * @param channel pointer to t_symbol of channel
 * @param track pointer to t_symbol of track
 */
void decrement(t_ml_counter *x, t_symbol *phrase, t_symbol *channel, t_symbol *track) {  
  output_value(x, --*get_counter(x, phrase, channel, track));
} 

/**
 * Resets counters value to zero for given phrase and channel
 *
 * @param x pointer to ml_counter object
 * @param phrase pointer to t_symbol of phrase
 * @param channel pointer to t_symbol of phrase
 */
void reset_channel_counters(t_ml_counter *x, t_symbol *phrase, t_symbol *channel) {
	for(int track_id = 1; track_id <= 4; track_id++) {
		char track_sym[3];
		sprintf(track_sym, "t%d", track_id);
		*get_counter(x, phrase, channel, gensym(track_sym)) = 0;
	}
}

/**
 * Resets counters value to zero for given phrase
 *
 * @param x pointer to ml_counter object
 * @param phrase pointer to t_symbol of phrase
 */
void reset_phrase_counters(t_ml_counter *x, t_symbol *phrase) {
	for(int channel_id = 1; channel_id <=2; channel_id++) {
		char channel_sym[4];
		sprintf(channel_sym, "ch%d", channel_id);
		reset_channel_counters(x, phrase, gensym(channel_sym));
	}
}

/**
 * Resets counter value to zero for given phrase, channel and track and sends it to outlet 
 *
 * @param x pointer to ml_counter object
 * @param phrase pointer to t_symbol holding phrase
 * @param channel pointer to t_symbol holding channel
 * @param track pointer to t_symbol holding track
 */
void reset(t_ml_counter *x, t_symbol *s, int argc, t_atom *argv) {  
	t_symbol *phrase;
	t_symbol *channel;
	t_symbol *track;
	
	switch(argc) {
		case 3:
			phrase = atom_getsymbol(argv);
			channel = atom_getsymbol(argv + 1);
			track = atom_getsymbol(argv + 2);
			*get_counter(x, phrase, channel, track) = 0;
			break;
		case 2:
			phrase = atom_getsymbol(argv);
			channel = atom_getsymbol(argv + 1);
			reset_channel_counters(x, phrase, channel);
			break;
		case 1:
			phrase = atom_getsymbol(argv);
			reset_phrase_counters(x, phrase);
			break;
		default:
			break;
	}
}
/*
void reset(t_ml_counter *x, t_symbol *phrase, t_symbol *channel, t_symbol *track) {  
  output_value(x, *get_counter(x, phrase, channel, track)=0);
} 

*/
/**
 * Instantiates ml_counter object
 */
void *ml_counter_new(void) {  
  t_ml_counter *x = (t_ml_counter *)pd_new(ml_counter_class);  
	
	// initialize counters to zeros 
	for(int i=0; i<2; i++) {
		for(int j=0; j<2; j++) {
			for(int k=0; k<4; k++) {
				x->counters[i][j][k] = 0;
			}	
		}
	}
		
	init_symb_id_map();

  //outlet_new(&x->x_obj, &s_float);
	outlet_new(&x->x_obj, &s_list);
  return (void *)x;  
}  

/**
 * Setup function for ml_counter class
 */
void ml_counter_setup(void) {  
  ml_counter_class = class_new(gensym("ml_counter"),  
        (t_newmethod)ml_counter_new,  
        0, sizeof(t_ml_counter),  
        CLASS_DEFAULT, 0);  

  class_addmethod(ml_counter_class, 
		(t_method)get,
		gensym("get"),
		A_SYMBOL,
		A_SYMBOL,
		A_SYMBOL, 0);

  class_addmethod(ml_counter_class,
  	(t_method)increment,
		gensym("incr"),
		A_SYMBOL,
		A_SYMBOL, 
		A_SYMBOL, 0);

  class_addmethod(ml_counter_class,
		(t_method)decrement,
		gensym("decr"),
		A_SYMBOL,
		A_SYMBOL,
		A_SYMBOL, 0);

  class_addmethod(ml_counter_class,
		(t_method)reset,
		gensym("reset"),
		A_GIMME, 0);
}
