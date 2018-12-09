#include "m_pd.h"  
#include "ml_definitions.h"
#include "ml_symb_id_map.h"
#include "ml_utils.h"

// allocation statuses 
enum {
    STOPPED,
    RUNNING,
    FLAGGED_STOP
};

typedef struct {
    t_symbol *phrase;
    t_symbol *channel;
    t_symbol *track;
    t_int layer;
    t_symbol *version;
    t_symbol *table_name;
} table;

/* NOTE: 
 * PD seems to produce audible glitches when table is resized by big number of samples. Because of this table is
 * at first allocated to small number of samples (N = 192 by default) and then dynamically reallocated every N samples by this number of
 * samples (when metro outside of this object sends bang). Simultaneously table size is recalculated and just stored to
 * table_size each beat to appropriate size (according to beat size in samples). When allocation stops table is resized to the size 
 * that was calculated and stored in table_size. However PD produces audible glitches  also when resizing multiple
 * tables by small number of samples at once. For this reason only a single table is reallocated every N samples while
 * tables are changing each turn. This means that each table is actually reallocated every N*num_of_tables_for_alloc samples 
 * and therefore tables are initially allocated to number of samples calculated as N*num_of_tables_for_alloc in order to
 * keep sufficient table size while table is waiting for its turn to be reallocated. 
 * 
 * By using this mechanism audio seems be free of glitches caused by reallocating at latency as low as 5 ms when running
 * on Raspberry Pi 3 with Audioinjector soundcard.
 */

// pointer to ml_table_allocator class
static t_class *ml_table_allocator_class;  
 
// data space for ml_table_allocator class
typedef struct _ml_table_allocator {  
    t_object                x_obj;  
    t_int                   tracks_sizes[2];            // 2 size holders for ALL tracks per phrase; 2 phrases x 1 tracks size
    
    t_int                   tmp_size;                   // dynamically allocated size is stored here, after allocation stops table is resized to calculated size
    t_int                   samples_per_resize;         // number of samples by which tmp_size is extended every time when resizing metro bangs

    t_int                   tempo;                      // tempo in bpm
    t_int                   beat_note_lengths[2];       // lengths of the beat notes (time signature denominator); 2 phrases x 1 beat note length;  whole=1, half=2, quarter=4
                                                        
    t_int                   beat_sizes[2];              // number of samples per 1 beat; 2 phrases x 1 beat size
    t_int                   sample_rate;                // samples per second

    t_int                   allocation_status;          // current status of allocation process  
    allocation_method       allocation_method;          // allocation method

    table                   tables_for_alloc[4];        // tables being allocated
    t_int                   num_of_tables_for_alloc;    // number of tables being allocated
    t_symbol                *phrases_for_alloc[2];      // all phrases to which allocated tables belong
    t_int                   num_of_phrases_for_alloc;   // number of all  phrases to which allocated tables belong

    t_int                   next_table_to_alloc;        /* on resizing metro bang always a single table is reallocated, if there are multiple tables for allocation 
                                                            each has its own turn and this variable holds the next table's id 
                                                            (this is because of performance reasons) */

    t_atom                  cmd_args[3];                // array of arguments for commands that are sent out of the object

    t_outlet                *cmd_out, *cmd_dest_out;    // outlets for commands sent out of the object and for symbols that sets their destination
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

void switch_resizing_metro(t_ml_table_allocator *x, t_int running) {
    outlet_symbol(x->cmd_dest_out, gensym("resizing_metro"));
    outlet_float(x->cmd_out, running);
}

/*******************************************************************************
 * allocator internal functions
 *******************************************************************************/

void resize_track(t_ml_table_allocator *x, t_symbol *table_name, t_int new_size) {
    t_garray *array = (t_garray *)pd_findbyclass(table_name, garray_class);
    garray_resize(array, new_size);
}

void stop_allocation(t_ml_table_allocator *x) {
    // stop metro that produces bangs to resize table
    switch_resizing_metro(x, 0);
    x->allocation_status = STOPPED;

    // in case when tables for 2 phrases are being allocated their size will be identical thus the size and beat count for the first phrase will be used
    t_int p_id = get_id_for_symb(x->phrases_for_alloc[0]);
    
    // after allocation stops resize tables to the size calculated by number of beats in the track
    for(int i = 0; i < x->num_of_tables_for_alloc; i++) {
        resize_track(x, x->tables_for_alloc[i].table_name, x->tracks_sizes[p_id]);
    }

    // reset counters
    x->num_of_tables_for_alloc = 0;
    x->num_of_phrases_for_alloc = 0;
    x->next_table_to_alloc = 0;
}

/*******************************************************************************
 * ml_table_allocator class methods
 *******************************************************************************/

void ml_table_allocator_start_allocation(t_ml_table_allocator *x, t_floatarg alloc_method) {
    x->allocation_status = RUNNING;

    // set tmp_size to 1 samples_per_resize so it will be alway one chunk size ahead
    x->tmp_size = x->num_of_tables_for_alloc * x->samples_per_resize;
    
    for(int i = 0; i < x->num_of_tables_for_alloc; i++) {
        resize_track(x, x->tables_for_alloc[i].table_name, x->tmp_size);
    }

    // turn on metro that produces bang to resize table every 192 samples
    switch_resizing_metro(x, 1);

    x->allocation_method = alloc_method;
}

void ml_table_allocator_flag_stop_allocation(t_ml_table_allocator *x) {
    x->allocation_status = FLAGGED_STOP;
}

void ml_table_allocator_bang(t_ml_table_allocator *x) {
    /* these are bangs produced by metro every N samples (defined by samples_per_resize), 
         resize track by adding number of samples stored in samples_per_resize on each bang 
         (this is because of performance reasons) */
    x->tmp_size += x->samples_per_resize;

    // resize next table
    resize_track(x, x->tables_for_alloc[x->next_table_to_alloc++].table_name, x->tmp_size);
    if(x->next_table_to_alloc == x->num_of_tables_for_alloc) {
        // the resized table was the last in the list, set next_table_to_alloc to the first one
        x->next_table_to_alloc = 0;
    }
}

void ml_table_allocator_add_table(t_ml_table_allocator *x, t_symbol *phrase, t_symbol *channel, t_symbol *track, t_floatarg layer, t_symbol *version) {
    table tbl = { 
        .phrase = phrase, 
        .channel = channel,
        .track = track,
        .layer = layer,
        .version = version,
        .table_name = get_table_name_symb(phrase, channel, track, layer, version)
    };
    x->tables_for_alloc[x->num_of_tables_for_alloc++] = tbl;

    if(x->num_of_phrases_for_alloc == 0) {
        // first phrase added, increment count and store it
        x->phrases_for_alloc[(x->num_of_phrases_for_alloc)++] = phrase;
    } else if(x->num_of_phrases_for_alloc == 1) {
        // there already is 1 phrase added
        if(x->phrases_for_alloc[x->num_of_phrases_for_alloc] != phrase) {
            // currently added phrase is different from the one already added, increment count and store it
            x->phrases_for_alloc[(x->num_of_phrases_for_alloc)++] = phrase;
        }
    }
}

void ml_table_allocator_set_tempo(t_ml_table_allocator *x, t_floatarg tempo) {
    x->tempo = tempo;
    recalc_beat_size(x, PHRASE_1);
    recalc_beat_size(x, PHRASE_2);
}

void ml_table_allocator_set_beat_note_length(t_ml_table_allocator *x, t_symbol *phrase, t_floatarg beat_note_length) {
    t_int p_id = get_id_for_symb(phrase);
    x->beat_note_lengths[p_id] = beat_note_length;
    recalc_beat_size(x, phrase);
}

void ml_table_allocator_new_beat(t_ml_table_allocator *x) {
    if(x->allocation_method == FREE_LENGTH) {
        if(x->allocation_status == RUNNING) {
            // add beat size to appropriate tracks size(s) and increment beat counter(s) during free-length allocation 
            for(int i = 0; i < x->num_of_phrases_for_alloc; i++) {
                t_int p_id = get_id_for_symb(x->phrases_for_alloc[i]);

                x->tracks_sizes[p_id] += x->beat_sizes[p_id];
            }
        }   
    }
}

void ml_table_allocator_set_up_new_cycle(t_ml_table_allocator *x) {
    // allocation always stops at the end of the cycle regardless when recording actually stops 
    if(x->allocation_status == RUNNING || x->allocation_status == FLAGGED_STOP) {
        stop_allocation(x);
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

    ml_table_allocator_set_beat_note_length(x, PHRASE_1, 4);
    ml_table_allocator_set_beat_note_length(x, PHRASE_2, 4);

    x->tracks_sizes[0] = 0;
    x->tracks_sizes[1] = 0;

    x->tmp_size = 0;
    x->samples_per_resize = 192;

    x->num_of_tables_for_alloc = 0;
    x->num_of_phrases_for_alloc = 0;
    
    x->next_table_to_alloc = 0;
    
    x->allocation_status = STOPPED;

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
        (t_method)ml_table_allocator_start_allocation,
        gensym(CMD_START_ALLOCATION),
        A_DEFFLOAT, 0);

    class_addmethod(ml_table_allocator_class,
        (t_method)ml_table_allocator_flag_stop_allocation,
        gensym(CMD_FLAG_STOP_ALLOCATION), 0);

    class_addbang(ml_table_allocator_class, ml_table_allocator_bang);

    class_addmethod(ml_table_allocator_class,
        (t_method)ml_table_allocator_add_table,
        gensym(CMD_ADD_TABLE),
        A_DEFSYMBOL,
        A_DEFSYMBOL,
        A_DEFSYMBOL,
        A_DEFFLOAT,
        A_DEFSYMBOL, 0);

    class_addmethod(ml_table_allocator_class, 
        (t_method)ml_table_allocator_set_tempo,
        gensym(CMD_SET_TEMPO),
        A_DEFFLOAT, 0);

    class_addmethod(ml_table_allocator_class, 
        (t_method)ml_table_allocator_set_beat_note_length,
        gensym(CMD_SET_BEAT_NOTE_LENGTH),
        A_DEFSYMBOL,
        A_DEFFLOAT, 0);

    class_addmethod(ml_table_allocator_class,
        (t_method)ml_table_allocator_new_beat,
        gensym(CMD_NEW_BEAT), 0);

    class_addmethod(ml_table_allocator_class,
        (t_method)ml_table_allocator_set_up_new_cycle,
        gensym(CMD_SET_UP_NEW_CYCLE), 0);
}
