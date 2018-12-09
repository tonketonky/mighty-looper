#include "m_pd.h"  
#include "symb_id_map.h"
#include "helpers_and_types.h"
#include "stdbool.h"
#include "stdlib.h"

// pointer to ml_player class
static t_class *ml_player_class;  
 
// data space for ml_player class
typedef struct _ml_player {  
    t_object    x_obj;  
    
    t_int       switch_looping_flags[2][4];         // flags for tracks to be switched from looping to not looping or vice versa on new cycle; 2 channels x 4 tracks
    t_int       track_muted_markers[2][2][4];       // markers indicating whether track is muted; 2 phrases x 2 channels x 2 tracks
    t_int       track_looping_markers[2][2][4];     // markers indicating whether track is looping; 2 phrases x 2 channels x 4 tracks

    t_symbol    *current_phrase;                    // current phrase within which the looper operates

    t_symbol    *channel_symbs[2];                  // symbols of all possible channels
    t_symbol    *track_symbs[4];                    // symbols of all possible tracks

    t_atom      cmd_args[3];                        // array of arguments for commands that are sent out of the object

    t_outlet    *cmd_out, *cmd_dest_out;            // outlets for commands sent out of the object and for symbols that sets their destination
} t_ml_player;  


/************************************************************
 * initialization functions
 ************************************************************/

void reset_switch_looping_flags(t_ml_player *x) {
    for(int ch_id = 0; ch_id < 2; ch_id++) {
        for(int t_id = 0; t_id < 4; t_id++) {
            x->switch_looping_flags[ch_id][t_id] = 0;
        }
    }
}

void reset_track_looping_markers(t_ml_player *x) {
    for(int p_id = 0; p_id < 2; p_id++) {
        for(int ch_id = 0; ch_id < 2; ch_id++) {
            for(int t_id = 0; t_id < 4; t_id++) {
                x->track_looping_markers[p_id][ch_id][t_id] = 0;
            }
        }
    }
}

void reset_track_muted_markers(t_ml_player *x) {
    for(int p_id = 0; p_id < 2; p_id++) {
        for(int ch_id = 0; ch_id < 2; ch_id++) {
            for(int t_id = 0; t_id < 4; t_id++) {
                x->track_muted_markers[p_id][ch_id][t_id] = 0;
            }
        }
    }
}

/******************************************************************
 * output commands functions
 ******************************************************************/

void set_looping(t_ml_player *x, t_symbol *channel, t_symbol *track, t_int is_looping) {
                
    char *channel_str = malloc(5);
    symb_2_string(channel, channel_str);
    char *track_str = malloc(5);
    symb_2_string(track, track_str);

    char *dest_str = malloc(20);

    sprintf(
        dest_str,
        "loop_%s_%s",
        channel_str,
        track_str
    );

    free(channel_str);
    free(track_str);

    t_symbol *dest = gensym(dest_str);
    free(dest_str);

    outlet_symbol(x->cmd_dest_out, dest);

    outlet_float(x->cmd_out, is_looping);
}

void notify_about_flagged_switch_looping(t_ml_player *x, t_symbol *channel, t_symbol *track, bool is_flagged) {
    // send command as first argument, message handler will transform it to message for gui
    if(is_flagged) {
        SETSYMBOL(x->cmd_args, gensym(CMD_TRACK_SWITCH_LOOPING_FLAGGED));
    } else {
        SETSYMBOL(x->cmd_args, gensym(CMD_TRACK_SWITCH_LOOPING_UNFLAGGED));
    }
    SETSYMBOL(x->cmd_args+1, channel);
    SETSYMBOL(x->cmd_args+2, track);

    t_symbol *cmd;
    cmd = gensym(CMD_SEND_TO_GUI);

    // notify gui
    outlet_symbol(x->cmd_dest_out, gensym("message_handler"));
    outlet_anything(x->cmd_out, cmd, 3, x->cmd_args);
}

void notify_about_switched_looping(t_ml_player *x, t_symbol *channel, t_symbol *track, bool is_looping) {
    // send command as first argument, message handler will transform it to message for gui
    if(is_looping) {
        SETSYMBOL(x->cmd_args, gensym(CMD_TRACK_LOOPING_STARTED));
    } else {
        SETSYMBOL(x->cmd_args, gensym(CMD_TRACK_LOOPING_STOPPED));
    }
    SETSYMBOL(x->cmd_args+1, channel);
    SETSYMBOL(x->cmd_args+2, track);

    t_symbol *cmd;
    cmd = gensym(CMD_SEND_TO_GUI);

    // notify gui
    outlet_symbol(x->cmd_dest_out, gensym("message_handler"));
    outlet_anything(x->cmd_out, cmd, 3, x->cmd_args);
}

/*******************************************************************************
 * ml_player class methods
 *******************************************************************************/

void ml_player_flag_switch_looping(t_ml_player *x, t_symbol *channel, t_symbol *track) {
    t_int ch_id = get_id_for_symb(channel);
    t_int t_id = get_id_for_symb(track);

    t_int *switch_looping_flag = &x->switch_looping_flags[ch_id][t_id];

    *switch_looping_flag = *switch_looping_flag ^ 1;
    notify_about_flagged_switch_looping(x, channel, track, *switch_looping_flag);
}

void ml_player_set_looping(t_ml_player *x, t_symbol *channel, t_symbol *track, t_floatarg is_looping) {
    t_int p_id = get_id_for_symb(x->current_phrase);
    t_int ch_id = get_id_for_symb(channel);
    t_int t_id = get_id_for_symb(track);

    x->track_looping_markers[p_id][ch_id][t_id] = is_looping;
    set_looping(x, channel, track, is_looping);
}

void ml_player_set_current_phrase(t_ml_player *x, t_symbol *phrase) {
    x->current_phrase = phrase;
}

void ml_player_set_up_new_cycle(t_ml_player *x) {
    t_int p_id = get_id_for_symb(x->current_phrase);
    
    // switch looping for all flagged tracks, notify gui
    for(t_int ch_id = 0; ch_id < 2; ch_id++) {
        for(t_int t_id = 0; t_id < 4; t_id++) {
            t_int *switch_looping_flag = &x->switch_looping_flags[ch_id][t_id];
            
            if(*switch_looping_flag == 1) {
                *switch_looping_flag = *switch_looping_flag ^ 1;
                
                t_int *track_looping_marker = &x->track_looping_markers[p_id][ch_id][t_id];
                *track_looping_marker = *track_looping_marker ^ 1;
                
                set_looping(x, x->channel_symbs[ch_id], x->track_symbs[t_id], *track_looping_marker);
                notify_about_switched_looping(x, x->channel_symbs[ch_id], x->track_symbs[t_id], *track_looping_marker);
            }
        }
    }
}

/**************************************************************
 * constructor
 **************************************************************/

void *ml_player_new(void) {  
    t_ml_player *x = (t_ml_player *)pd_new(ml_player_class);  

    reset_switch_looping_flags(x);
    reset_track_looping_markers(x);
    reset_track_muted_markers(x);

    init_symb_id_map();
    
    x->current_phrase = PHRASE_1;

    x->channel_symbs[0] = gensym("ch1");
    x->channel_symbs[1] = gensym("ch2");
    x->track_symbs[0] = gensym("t1");
    x->track_symbs[1] = gensym("t2");
    x->track_symbs[2] = gensym("t3");
    x->track_symbs[3] = gensym("t4");

    // outlets
    x->cmd_out = outlet_new(&x->x_obj, 0);
    x->cmd_dest_out = outlet_new(&x->x_obj, &s_symbol);
    return (void *)x;  
}  

/**************************************************************
 * destructor
 **************************************************************/

void ml_player_free(t_ml_player *x) {
    outlet_free(x->cmd_out);
    outlet_free(x->cmd_dest_out);
}

/**************************************************************
 * Setup function for ml_player class
 **************************************************************/

void ml_player_setup(void) {  
    ml_player_class = class_new(gensym("ml_player"),  
        (t_newmethod)ml_player_new,  
        (t_method)ml_player_free, 
        sizeof(t_ml_player),  
        CLASS_DEFAULT, 0);  

    class_addmethod(ml_player_class, 
        (t_method)ml_player_flag_switch_looping,
        gensym(CMD_TRACK_FLAG_SWITCH_LOOPING),
        A_DEFSYMBOL,
        A_DEFSYMBOL, 0);

    class_addmethod(ml_player_class, 
        (t_method)ml_player_set_looping,
        gensym(CMD_TRACK_SET_LOOPING),
        A_DEFSYMBOL,
        A_DEFSYMBOL,
        A_DEFFLOAT, 0);

    class_addmethod(ml_player_class, 
        (t_method)ml_player_set_current_phrase,
        gensym(CMD_SET_CURRENT_PHRASE),
        A_DEFSYMBOL, 0);

    class_addmethod(ml_player_class, 
        (t_method)ml_player_set_up_new_cycle,
        gensym(CMD_SET_UP_NEW_CYCLE), 0);
}
