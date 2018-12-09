#include "m_pd.h"  
#include "ml_definitions.h"
#include "ml_symb_id_map.h"
#include "ml_utils.h"

// pointer to ml_layer_merger class
static t_class *ml_layer_merger_class;  
 
// data space for ml_layer_merger class
typedef struct _ml_layer_merger {  
    t_object                x_obj;  

    t_int                   is_flagged;                         // indicates whether layers are flagged for merging

    t_symbol                *flagged_phrase;                    // phrase flagged for merging to
    t_symbol                *flagged_channel;                   // channel flagged for merging to
    t_symbol                *flagged_track;                     // track flagged for merging to
    t_symbol                *flagged_version;                   // version flagged for merging to

    t_int                   flagged_num_of_layers_to_merge;     // number of layer to be merged

    allocation_method       flagged_alloc_method;               // allocation method for table flagged for merging to

    t_atom                  cmd_args[5];                        // array of arguments for commands that are sent out of the object

    t_outlet                *cmd_out, *cmd_dest_out;            // outlets for commands sent out of the object and for symbols that sets their destination
} t_ml_layer_merger;  


/******************************************************************
 * layers merging management functions
 ******************************************************************/

void flag_merging(t_ml_layer_merger *x, t_symbol *phrase, t_symbol *channel, t_symbol *track, t_symbol *version, t_int num_of_layers_to_merge, allocation_method alloc_method) {
    x->is_flagged = 1;
    x->flagged_alloc_method = alloc_method;
    
    x->flagged_num_of_layers_to_merge = num_of_layers_to_merge;

    x->flagged_phrase = phrase;
    x->flagged_channel = channel;
    x->flagged_track = track;
    x->flagged_version = version;
}

/********************************************************************
 * output commands functions
 ********************************************************************/

void start_merging(t_ml_layer_merger *x) {
    
    
    outlet_symbol(x->cmd_dest_out, gensym("tabwrite_merge"));

    SETSYMBOL(
        x->cmd_args,
        get_table_name_symb(
            x->flagged_phrase, 
            x->flagged_channel, 
            x->flagged_track,
            1,
            x->flagged_version
        )
    );
    
    outlet_anything(x->cmd_out, gensym("set"), 1, x->cmd_args);

    outlet_symbol(x->cmd_dest_out, gensym("tabplay_merge_1"));

    outlet_anything(x->cmd_out, gensym("stop"), 0, 0);

    SETSYMBOL(
        x->cmd_args,
        get_table_name_symb(
            x->flagged_phrase, 
            x->flagged_channel, 
            x->flagged_track,
            1,
            get_opp_version(x->flagged_version) 
        )
    );
    
    outlet_anything(x->cmd_out, gensym("set"), 1, x->cmd_args);


    outlet_symbol(x->cmd_dest_out, gensym("tabplay_merge_2"));

    outlet_anything(x->cmd_out, gensym("stop"), 0, 0);

    if(x->flagged_num_of_layers_to_merge == 2) {
    
        SETSYMBOL(
            x->cmd_args,
            get_table_name_symb(
                x->flagged_phrase, 
                x->flagged_channel, 
                x->flagged_track,
                2,
                get_opp_version(x->flagged_version) 
            )
        );
    
        outlet_anything(x->cmd_out, gensym("set"), 1, x->cmd_args);
    } else {
        outlet_anything(x->cmd_out, gensym("set"), 0, 0);
    }

    outlet_symbol(x->cmd_dest_out, gensym("start_merging"));
    outlet_bang(x->cmd_out);

    x->is_flagged = 0;
}

/*******************************************************************************
 * ml_layer_merger class methods
 *******************************************************************************/

void ml_layer_merger_flag_merging(t_ml_layer_merger *x, t_symbol *s, int argc, t_atom *argv) {  

    t_symbol *phrase = atom_getsymbol(argv);
    t_symbol *channel = atom_getsymbol(argv+1);
    t_symbol *track = atom_getsymbol(argv+2);
    t_symbol *version = atom_getsymbol(argv+3);
    t_int num_of_layers_to_merge = atom_getint(argv+4);
    allocation_method alloc_method = atom_getint(argv+5);

    if(x->is_flagged == 0) {
        flag_merging(x, phrase, channel, track, version, num_of_layers_to_merge, alloc_method);
    } else{
        if(x->flagged_phrase == phrase && x->flagged_channel == channel && x->flagged_track == track) {
            x->is_flagged = 0;
        } else {
            flag_merging(x, phrase, channel, track, version, num_of_layers_to_merge, alloc_method);
        }
    }
}

void ml_layer_merger_set_up_new_cycle(t_ml_layer_merger *x) {
    if(x->is_flagged == 1 && x->flagged_alloc_method != NONE) {
        // allocation method is not "none" -> allocation will be needed
        outlet_symbol(x->cmd_dest_out, gensym("table_allocator"));

        // add table to allocator so it will be allocated when allocation starts
        SETSYMBOL(x->cmd_args, x->flagged_phrase);
        SETSYMBOL(x->cmd_args+1, x->flagged_channel);
        SETSYMBOL(x->cmd_args+2, x->flagged_track);
        SETFLOAT(x->cmd_args+3, 1);
        SETSYMBOL(x->cmd_args+4, x->flagged_version);

        outlet_anything(x->cmd_out, gensym(CMD_ADD_TABLE), 5, x->cmd_args);
    }
}

void ml_layer_merger_new_cycle(t_ml_layer_merger *x) {
    if(x->is_flagged == 1) {
        start_merging(x);
    }   
}

/**************************************************************
 * constructor
 **************************************************************/

void *ml_layer_merger_new(void) {  
    t_ml_layer_merger *x = (t_ml_layer_merger *)pd_new(ml_layer_merger_class);  
    
    init_symb_id_map();
    
    x->is_flagged = 0;

    // outlets
    x->cmd_out = outlet_new(&x->x_obj, 0);
    x->cmd_dest_out = outlet_new(&x->x_obj, &s_symbol);
    
    return (void *)x;  
}  

/**************************************************************
 * destructor
 **************************************************************/

void ml_layer_merger_free(t_ml_layer_merger *x) {
    outlet_free(x->cmd_out);
    outlet_free(x->cmd_dest_out);
}

/**************************************************************
 * Setup function for ml_layer_merger class
 **************************************************************/

void ml_layer_merger_setup(void) {  
    ml_layer_merger_class = class_new(gensym("ml_layer_merger"),  
        (t_newmethod)ml_layer_merger_new,  
        (t_method)ml_layer_merger_free,
        sizeof(t_ml_layer_merger),  
        CLASS_DEFAULT, 0);  

    class_addmethod(ml_layer_merger_class, 
        (t_method)ml_layer_merger_flag_merging,
        gensym(CMD_FLAG_MERGING),
        A_GIMME, 0);

    class_addmethod(ml_layer_merger_class, 
        (t_method)ml_layer_merger_set_up_new_cycle,
        gensym(CMD_SET_UP_NEW_CYCLE), 0);

    class_addmethod(ml_layer_merger_class, 
        (t_method)ml_layer_merger_new_cycle,
        gensym(CMD_NEW_CYCLE), 0);
}
