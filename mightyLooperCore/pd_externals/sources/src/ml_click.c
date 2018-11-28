#include "m_pd.h"  
#include "symb_id_map.h"
#include "helpers_and_types.h"
#include "stdlib.h"

// pointer to ml_click class
static t_class *ml_click_class;  
 
// data space for ml_click class
typedef struct _ml_click {  
    t_object  x_obj;  
    
    t_int       accent_tone_freq;                   // frequency of accented click tone
    t_int       normal_tone_freq;                   // frequency of non-accented click tone

    t_int       click_state;                        // indicates whether click is turned ON/OFF
    
    t_int       tempo;                              // tempo in bpm (quarter note beats per minute)
    
    t_int       time_signatures[2][2];              // time signatures for each phrase; 2 phrases x (nominator + denominator)

    t_symbol    *phrases_for_counting_beats[2];     // list of phrases for which beats are counted 
    t_int       num_of_phrases_for_counting_beats;  // number of phrases in list for counting beats
    t_int       is_counting_beats;                  // indicates whether counting beats is turned ON/OFF

    t_int       beat_counts[2];                     // number of beats per cycle for phrase; 2 phrases x number of beats

    t_int       current_beat;                       // current beat number

    t_symbol    *current_phrase;                    // the phrase for which click clicks

    t_atom      cmd_args[3];                        // array of arguments for commands that are sent out of the object

    t_outlet    *cmd_out, *cmd_dest_out;            // outlets for commands sent out of the object and for symbols that sets their destination
} t_ml_click;  


/******************************************************************
 * output commands functions
 ******************************************************************/

void set_click_tone(t_ml_click *x, t_int freq) {
    outlet_symbol(x->cmd_dest_out, gensym("click_tone"));
    outlet_float(x->cmd_out, freq);
}

void click_metro_set_tempo(t_ml_click *x, t_int tempo) {
    outlet_symbol(x->cmd_dest_out, gensym("click_metro"));

    SETFLOAT(x->cmd_args, tempo);
    SETSYMBOL(x->cmd_args+1, gensym("permin"));

    outlet_anything(x->cmd_out, gensym("tempo"), 2, x->cmd_args);
}

void click_metro_switch(t_ml_click *x, t_int state) {
    outlet_symbol(x->cmd_dest_out, gensym("click_metro"));
    outlet_float(x->cmd_out, state);
}

void table_allocator_set_tempo(t_ml_click *x, t_int tempo) {
    outlet_symbol(x->cmd_dest_out, gensym("table_allocator"));

    SETFLOAT(x->cmd_args, tempo);

    outlet_anything(x->cmd_out, gensym("set_tempo"), 1, x->cmd_args);
}

void new_bar(t_ml_click *x) {
    outlet_symbol(x->cmd_dest_out, gensym("new_bar"));
    outlet_anything(x->cmd_out, gensym("new_bar"), 0, 0);
}

void new_beat(t_ml_click *x) {
    outlet_symbol(x->cmd_dest_out, gensym("new_beat"));
    outlet_anything(x->cmd_out, gensym("new_beat"), 0, 0);
}

void set_up_new_cycle(t_ml_click *x) {
    outlet_symbol(x->cmd_dest_out, gensym("set_up_new_cycle"));
    outlet_anything(x->cmd_out, gensym("set_up_new_cycle"), 0, 0);
}

void new_cycle(t_ml_click *x) {
    outlet_symbol(x->cmd_dest_out, gensym("new_cycle"));
    outlet_anything(x->cmd_out, gensym("new_cycle"), 0, 0);
}

/********************************************************************
 * data space values getters and setters
 ********************************************************************/

void set_time_signature(t_ml_click *x, t_symbol *phrase, t_int nominator, t_int denominator) {
    int p_id = get_id_for_symb(phrase); 
    x->time_signatures[p_id][0] = nominator;
    x->time_signatures[p_id][1] = denominator;
}

void set_beat_count(t_ml_click *x, t_symbol *phrase, t_int num_of_beats) {
    int p_id = get_id_for_symb(phrase);
    x->beat_counts[p_id] = num_of_beats;
}

void set_tempo(t_ml_click *x, t_int tempo) {
    x->tempo = tempo;
    click_metro_set_tempo(x, tempo);
    table_allocator_set_tempo(x, tempo);
}
/************************************************************
 * initialization functions
 ************************************************************/

void reset_click_for_phrase(t_ml_click *x, t_symbol *phrase) {
    set_time_signature(x, phrase, 4, 4);
    set_beat_count(x, phrase, 0);
}

void reset_click(t_ml_click *x) {
    x->current_phrase = PHRASE_1;
    reset_click_for_phrase(x, PHRASE_1);
    reset_click_for_phrase(x, PHRASE_2);
    set_tempo(x, 60);
    x->current_beat = 1;
}

/*******************************************************************************
 * ml_click class methods
 *******************************************************************************/

void ml_click_bang(t_ml_click *x) {
    int p_id = get_id_for_symb(x->current_phrase);
    
    if(x->beat_counts[p_id] != 0 && x->is_counting_beats == 0) {
        // beat count for the phrase is already set
        if((x->current_beat) > x->beat_counts[p_id]) {
            // current beat number exceeded  current phrase beat count, reset it to 1 (first beat of cycle)
            x->current_beat = 1;
        }
        if(x->current_beat == 1) {
            // current beat is the first beat of cycle, send set_up_new_cycle and new_cycle
            set_up_new_cycle(x);
            new_cycle(x);
        }
    }

    t_int nominator = x->time_signatures[p_id][0];
    if((x->current_beat - 1 + nominator) % nominator == 0) {
        /* first beat of a bar, set accent tone and send new_bar
             e.g. 
             with 4/4 time signature and first beat -> (1 - 1 + 4) % 4 == 0
             with 4/4 time signature and fifth beat -> (5 - 1 + 4) % 4 == 0
        */
        set_click_tone(x, x->accent_tone_freq);
        new_bar(x);
    } else {
        /* not first beat of a bar, set normal tone
             e.g. 
             with 4/4 time signature and second beat -> (1 -1 + 4) % 4 != 0
             with 4/4 time signature and seventh beat -> (6 - 1 + 4) % 4 != 0
        */
        set_click_tone(x, x->normal_tone_freq);
    }

    if(x->is_counting_beats == 1) {
        // if counting beats is turned on increment beat count for each phrase set for counting beats
        for(int i = 0; i < x->num_of_phrases_for_counting_beats; i++) {
            x->beat_counts[get_id_for_symb(x->phrases_for_counting_beats[i])]++;
        }
    }
    // send new_beat
    new_beat(x);

    // increment current beat number for next bang
    x->current_beat++;
}

void ml_click_switch(t_ml_click *x, t_floatarg state) {
    x->click_state = state;
    click_metro_switch(x, state);
}

void ml_click_set_tempo(t_ml_click *x, t_floatarg tempo) {
    set_tempo(x, tempo);
}

void ml_click_set_time_signature(t_ml_click *x, t_symbol *phrase, t_floatarg nominator, t_floatarg denominator) {
    set_time_signature(x, phrase, nominator, denominator);
}

void ml_click_start_counting_beats(t_ml_click *x, t_symbol *s, int argc, t_atom *argv) {
    // set number of phrases for counting beats to number of arguments given
    x->num_of_phrases_for_counting_beats = argc;
    for(int i = 0; i < argc; i++) {
        // set given phrases to array and reset its beat count
        x->phrases_for_counting_beats[i] = atom_getsymbol(argv + i);
        x->beat_counts[get_id_for_symb(x->phrases_for_counting_beats[i])] = 0;
    }   
    // turn on counting beats
    x->is_counting_beats = 1;
}

void ml_click_stop_counting_beats(t_ml_click *x) {
    // reset beat number to 1 (first beat of cycle) and turn off counting beats 
    x->current_beat = 1;
    x->is_counting_beats = 0;
}

void ml_click_reset(t_ml_click *x, t_symbol *s, int argc, t_atom *argv) {
    if(argc == 0) {
        // reset entire click
        reset_click(x);
    } else {
        // reset click for given phrase
        reset_click_for_phrase(x, atom_getsymbol(argv));
    }
}

/**************************************************************
 * constructor
 **************************************************************/

void *ml_click_new(void) {  
    t_ml_click *x = (t_ml_click *)pd_new(ml_click_class);  

    init_symb_id_map();

    x->accent_tone_freq = 1000;
    x->normal_tone_freq = 800;

    x->is_counting_beats = 0;

    // outlets
    x->cmd_out = outlet_new(&x->x_obj, 0);
    x->cmd_dest_out = outlet_new(&x->x_obj, &s_symbol);

    return (void *)x;  
}  

/**************************************************************
 * destructor
 **************************************************************/

void ml_click_free(t_ml_click *x) {
    outlet_free(x->cmd_out);
    outlet_free(x->cmd_dest_out);
}

/**************************************************************
 * Setup function for ml_click class
 **************************************************************/

void ml_click_setup(void) {  
    ml_click_class = class_new(gensym("ml_click"),  
        (t_newmethod)ml_click_new,  
        (t_method)ml_click_free, 
        sizeof(t_ml_click),  
        CLASS_DEFAULT, 0);  

    class_addbang(ml_click_class,
        (t_method)ml_click_bang);

    class_addmethod(ml_click_class, 
        (t_method)ml_click_switch,
        gensym("switch"),
        A_DEFFLOAT, 0);

    class_addmethod(ml_click_class,
        (t_method)ml_click_set_tempo,
        gensym("set_tempo"),
        A_DEFFLOAT, 0);

    class_addmethod(ml_click_class,
        (t_method)ml_click_set_time_signature,
        gensym("set_time_signature"),
        A_DEFSYMBOL,
        A_DEFFLOAT,
        A_DEFFLOAT, 0);

    class_addmethod(ml_click_class,
        (t_method)ml_click_start_counting_beats,
        gensym("start_counting_beats"),
        A_GIMME, 0);

    class_addmethod(ml_click_class,
        (t_method)ml_click_stop_counting_beats,
        gensym("stop_counting_beats"), 0);

    class_addmethod(ml_click_class,
        (t_method)ml_click_reset,
        gensym("reset"),
        A_GIMME, 0);
}
