#include "m_pd.h"
#include "evt_cmd_map.h"
#include "cmd_dest_map.h"
#include "symb_id_map.h"
#include "helpers_and_types.h"
#include "stdlib.h"
#include "string.h"

// pointer to ml_message_handler class
static t_class *ml_message_handler_class;

// data space for ml_message_handler class
typedef struct _ml_message_handler {
  t_object  x_obj;

	t_int  		is_recording_markers[2][4]; 			// indicates whether track is being recorded; 2 channels x 4 tracks

	t_int 		is_processing;

	char 			buf[8];
	char 			*event_code;

	t_int 		num_of_chars_processed;
	t_int 		num_of_literals_processed;

	t_atom 		cmd_args[4]; 											// array of arguments for commands that are sent out of the object

	t_outlet 	*cmd_out, *cmd_dest_out; 					// outlets for commands sent out of the object and for symbols that sets their destination
} t_ml_message_handler;

/*******************************************************************************
 * initialization functions
 *******************************************************************************/

void reset_is_recording_markers(t_ml_message_handler *x) {
	for(int ch_id = 0; ch_id < 2; ch_id++) {
		for(int t_id = 0; t_id < 4; t_id++) {
			x->is_recording_markers[ch_id][t_id] = 0;
		}
	}
}

/********************************************************************
 * data space member getters
 ********************************************************************/

t_int *get_is_recording_marker(t_ml_message_handler *x, t_symbol *channel, t_symbol *track) {
	int channel_id = get_id_for_symb(channel);
	int track_id = get_id_for_symb(track);

	return &(x->is_recording_markers[channel_id][track_id]);
}

/*******************************************************************************
 * ml_message_handler class methods
 *******************************************************************************/

void ml_message_handler_process_input(t_ml_message_handler *x, t_floatarg receivedChar) {
  // TODO: reimplement using regex
  // handler is not processing input and received char is beginning of the message
	if(x->is_processing == 0 && receivedChar == '[') {
		x->is_processing = 1;
	} else {
		if(x->is_processing == 1) {
			if(receivedChar != ']' && receivedChar != '/') {
			  // is not delimiter (slash) nor end of message (']'), append char to buf
        x->buf[x->num_of_chars_processed++] = receivedChar;
			}
			if(receivedChar == '/' || receivedChar == ']') {
			    // is delimiter (slash) or end of message (']'), terminate string in buf with null
				x->buf[x->num_of_chars_processed] = '\0';
				// reset char counter
				x->num_of_chars_processed = 0;

				if(x->num_of_literals_processed == 0) {
					// currently processed literal is event code, save it to x->event_code
					x->event_code = strdup(x->buf);
			    } else {
					// currently processed literal is argument, add it to atom list x->cmd_args
            if(x->buf[0] == '#') {
              // numeric argument
              SETFLOAT(x->cmd_args + x->num_of_literals_processed - 1, atoi(x->buf+1));
            } else if(x->buf[0] == '$') {
              // string argument
              SETSYMBOL(x->cmd_args + x->num_of_literals_processed - 1, gensym(x->buf+1));
            }
				}
				// increment literal counter
				x->num_of_literals_processed++;
			}
			if(receivedChar == ']') {
				// end of processing message
				char *cmd;

				// interpretation of messages from record buttons depends on whether given track is currently being recorded,
				// check the first char of event code, if it's 'r', append recording flag to it and get command
				// otherwise just get command for the event code as is
        if(x->event_code[0] == 'r') {
          // event code is from record button, get first 2 arguments from atom list x->cmd_args (channel and track)
          // find out whether given track is being recorded and get command accordingly
          int ch_id = get_id_for_symb(atom_gensym(&x->cmd_args[0]));
          int t_id = get_id_for_symb(atom_gensym(&x->cmd_args[1]));
          char *rec_specific_event_code = malloc(sizeof(x->event_code) + 4);
          sprintf(rec_specific_event_code, "%s_r=%d", x->event_code, x->is_recording_markers[ch_id][t_id]);
          cmd = get_cmd_for_evt(rec_specific_event_code);
          free(rec_specific_event_code);
        } else {
          // event code is from play button or click setting, get command for it
          cmd = get_cmd_for_evt(x->event_code);
				}

				if(cmd != NULL) {
					// command is defined for given event code, get destination and output it along with arguments
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

void ml_message_handler_recording_started(t_ml_message_handler *x, t_symbol *channel, t_symbol *track) {
	*get_is_recording_marker(x, channel, track) = 1;
}

void ml_message_handler_recording_stopped(t_ml_message_handler *x, t_symbol *channel, t_symbol *track) {
	*get_is_recording_marker(x, channel, track) = 0;
}

/**************************************************************
 * constructor
 **************************************************************/

void *ml_message_handler_new(void) {
  t_ml_message_handler *x = (t_ml_message_handler *)pd_new(ml_message_handler_class);

	init_evt_cmd_map();
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

void ml_message_handler_free(t_ml_message_handler *x) {
	outlet_free(x->cmd_out);
	outlet_free(x->cmd_dest_out);
}

/**************************************************************
 * Setup function for ml_message_handler class
 **************************************************************/

void ml_message_handler_setup(void) {
  ml_message_handler_class = class_new(gensym("ml_message_handler"),
        (t_newmethod)ml_message_handler_new,
        (t_method)ml_message_handler_free,
				sizeof(t_ml_message_handler),
        CLASS_DEFAULT, 0);

  class_addfloat(ml_message_handler_class,
		(t_method)ml_message_handler_process_input);

	class_addmethod(ml_message_handler_class,
		(t_method)ml_message_handler_recording_started,
		gensym("recording_started"),
		A_DEFSYMBOL,
		A_DEFSYMBOL, 0);

	class_addmethod(ml_message_handler_class,
		(t_method)ml_message_handler_recording_stopped,
		gensym("recording_stopped"),
		A_DEFSYMBOL,
		A_DEFSYMBOL, 0);
}
