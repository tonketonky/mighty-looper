#include "m_pd.h" 
#include "stdlib.h"

#define PHRASE1 gensym("p1")
#define PHRASE2 gensym("p2")
#define VERSION_A gensym("a")
#define VERSION_B gensym("b")

/*****************************************************************
 * Helper functions
 *****************************************************************/

t_symbol *get_opp_phrase(t_symbol *phrase) {
	return phrase == PHRASE1 ? PHRASE2 : PHRASE1;
}

t_symbol *get_opp_version(t_symbol *version) {
	return version == VERSION_A ? VERSION_B : VERSION_A;
}

void symb_2_string(t_symbol *symbol, char *string) {
	t_atom *at = malloc(sizeof(t_atom));
	SETSYMBOL(at, symbol);
	atom_string(at, string, 5);
	free(at);
}

t_symbol *get_table_name_symb(t_symbol *phrase, t_symbol *channel, t_symbol *track, t_int layer, t_symbol *version) {

	char *phrase_str = malloc(5);
	symb_2_string(phrase, phrase_str);
	char *channel_str = malloc(5);
	symb_2_string(channel, channel_str);
	char *track_str = malloc(5);
	symb_2_string(track, track_str);
	char *version_str = malloc(5);
	symb_2_string(version, version_str);


	char *table_name_str = malloc(25);

	sprintf(
		table_name_str,
		"%s_%s_%s_l%lu_%s",
		//"%s_%s_%s_%s",
		phrase_str,
		channel_str,
		track_str,
		layer,
		version_str
	);

	free(phrase_str);
	free(channel_str);
	free(track_str);
	free(version_str);

	t_symbol *table_name_symb = gensym(table_name_str);
	free(table_name_str);

	return table_name_symb;
}


