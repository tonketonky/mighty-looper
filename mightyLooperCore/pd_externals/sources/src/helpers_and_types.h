#define PHRASE_1 gensym("p1")
#define PHRASE_2 gensym("p2")
#define VERSION_A gensym("a")
#define VERSION_B gensym("b")

typedef enum{
    FREE_LENGTH,
    FIXED_LENGTH,
    NONE
} allocation_method;

t_symbol *get_opp_phrase(t_symbol *phrase);
t_symbol *get_opp_version(t_symbol *version);
void symb_2_string(t_symbol *symbol, char *string);
t_symbol *get_table_name_symb(t_symbol *phrase, t_symbol *channel, t_symbol *track, t_int layer, t_symbol *version);
