typedef enum{
	FREE_LENGTH,
	FIXED_LENGTH,
	NONE
} allocation_method;

t_symbol *get_opp_phrase(t_symbol *phrase);
t_symbol *get_opp_version(t_symbol *version);
void symb_2_string(t_symbol *symbol, char *string);
t_symbol *get_table_name_symb(t_symbol *phrase, t_symbol *channel, t_symbol *track, t_int layer, t_symbol *version);
