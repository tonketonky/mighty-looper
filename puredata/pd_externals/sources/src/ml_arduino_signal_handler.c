#include "m_pd.h"  
#include "sig_cmd_map.h"
#include "cmd_dest_map.h"
#include "symb_id_map.h"
#include "helpers.h"
#include "stdlib.h"
#include "string.h"

#define PHRASE1 gensym("p1")
#define PHRASE2 gensym("p2")

// pointer to ml_arduino_signal_handler class
static t_class *ml_arduino_signal_handler_class;  
 
// data space for ml_arduino_signal_handler class
typedef struct _ml_arduino_signal_handler {  
  t_object  x_obj;  

	t_int  		is_recording_markers[2][4]; 			// indicates whether track is being recorded; 2 channels x 4 tracks

	t_int 		is_processing;

	char 			buf[8];
	char 			*cmd_signal;

	t_int 		num_of_chars_processed;
	t_int 		num_of_literals_processed;

	t_atom 		cmd_args[4]; 											// array of arguments for commands that are sent out of the object

	t_outlet 	*cmd_out, *cmd_dest_out; 					// outlets for commands sent out of the object and for symbols that sets their destination
} t_ml_arduino_signal_handler;  

/*******************************************************************************
 * initialization functions
 *******************************************************************************/

void reset_is_recording_markers(t_ml_arduino_signal_handler *x) {
	for(int ch_id = 0; ch_id < 2; ch_id++) {
		for(int t_id = 0; t_id < 4; t_id++) {
			x->is_recording_markers[ch_id][t_id] = 0;
		}
	}
}

/********************************************************************
 * data space member getters
 ********************************************************************/

t_int *get_is_recording_marker(t_ml_arduino_signal_handler *x, t_symbol *channel, t_symbol *track) {
	int channel_id = get_id_for_symb(channel);
	int track_id = get_id_for_symb(track);

	return &(x->is_recording_markers[channel_id][track_id]);
}

/*******************************************************************************
 * ml_arduino_signal_handler class methods
 *******************************************************************************/

void ml_arduino_signal_handler_process_input(t_ml_arduino_signal_handler *x, t_floatarg signal) {
	if(x->is_processing == 0 && signal == '[') {
		x->is_processing = 1;
	} else {
		if(x->is_processing == 1) {
			if(signal != ']' && signal != ' ') {
				x->buf[x->num_of_chars_processed++] = signal;
			}
			if(signal == ' ' || signal == ']') {
				x->buf[x->num_of_chars_processed] = '\0';
				x->num_of_chars_processed = 0;
			
				if(x->num_of_literals_processed == 0) {
					// currently processed literal is signal for command, save it to x->cmd_signal
					x->cmd_signal = strdup(x->buf);
			} else {
					// currently processed literal is signal for command argument, add it to atom list x->cmd_args
					SETSYMBOL(x->cmd_args + x->num_of_literals_processed - 1, gensym(x->buf));
				}
				x->num_of_literals_processed++;
			}
			if(signal == ']') {
				// end of processing signals
					char *cmd;
					if(x->cmd_signal[0] == 'r') {
						// command signal is from record button, get first 2 arguments from atom list x->cmd_args (channel and track)
						// find out whether given track is being recorded and get command accordingly
						int ch_id = get_id_for_symb(atom_gensym(&x->cmd_args[0]));
						int t_id = get_id_for_symb(atom_gensym(&x->cmd_args[1]));
						cmd = get_cmd_for_rec_sig(x->cmd_signal, x->is_recording_markers[ch_id][t_id]);
					} else {
						// command signal is from play button, get command for it
						cmd = get_cmd_for_play_sig(x->cmd_signal);
					}
				
				if(cmd != NULL) {
					// command is defined for given signal, get destination and output it along with arguments
					t_symbol *dest = gensym(get_dest_for_cmd(cmd));
					outlet_symbol(x->cmd_dest_out, dest);
					outlet_anything(x->cmd_out, gensym(cmd), x->num_of_literals_processed - 1, x->cmd_args);
				}
				// reset variables for next processing 
				x->num_of_literals_processed = 0;
				x->is_processing = 0;
			}
		}
	}
}

void ml_arduino_signal_handler_recording_started(t_ml_arduino_signal_handler *x, t_symbol *channel, t_symbol *track) {
	*get_is_recording_marker(x, channel, track) = 1;
}

void ml_arduino_signal_handler_recording_stopped(t_ml_arduino_signal_handler *x, t_symbol *channel, t_symbol *track) {
	*get_is_recording_marker(x, channel, track) = 0;
}

/**************************************************************
 * constructor
 **************************************************************/

void *ml_arduino_signal_handler_new(void) {  
  t_ml_arduino_signal_handler *x = (t_ml_arduino_signal_handler *)pd_new(ml_arduino_signal_handler_class);  

	init_sig_cmd_map();
	init_cmd_dest_map();
	init_symb_id_map();

	reset_is_recording_markers(x);

	x->is_processing = 0;
	x->num_of_chars_processed = 0;
	x->num_of_literals_processed = 0;

	// outlets
	x->cmd_out = outlet_new(&x->x_obj, 0);
	x->cmd_dest_out = outlet_new(&x->x_obj, &s_symbol);
	return (void *)x;  
}  

/**************************************************************
 * destructor
 **************************************************************/

void ml_arduino_signal_handler_free(t_ml_arduino_signal_handler *x) {
	outlet_free(x->cmd_out);
	outlet_free(x->cmd_dest_out);
}

/**************************************************************
 * Setup function for ml_arduino_signal_handler class
 **************************************************************/

void ml_arduino_signal_handler_setup(void) {  
  ml_arduino_signal_handler_class = class_new(gensym("ml_arduino_signal_handler"),  
        (t_newmethod)ml_arduino_signal_handler_new,  
        (t_method)ml_arduino_signal_handler_free, 
				sizeof(t_ml_arduino_signal_handler),  
        CLASS_DEFAULT, 0);  

  class_addfloat(ml_arduino_signal_handler_class, 
		(t_method)ml_arduino_signal_handler_process_input);

	class_addmethod(ml_arduino_signal_handler_class,
		(t_method)ml_arduino_signal_handler_recording_started,
		gensym("recording_started"),
		A_DEFSYMBOL,
		A_DEFSYMBOL, 0);

	class_addmethod(ml_arduino_signal_handler_class,
		(t_method)ml_arduino_signal_handler_recording_stopped,
		gensym("recording_stopped"),
		A_DEFSYMBOL,
		A_DEFSYMBOL, 0);
}
