#include "m_pd.h"  
#include "search.h"
#include "stdlib.h"
#include "string.h"
#include "stdio.h"

/* data structs and vars for map implementation - begin */
typedef struct {
    t_symbol *key;
    int value;
} symb_id_map;

void *symb_id_map_root;
symb_id_map *symb_id_map_find;
/* data structs and vars for map implementation - end */

/**
 * Compares 2 pd symbols (t_symbol types)
 * 
 * @param s1 pointer to first t_symbol to symb_id_map_compare
 * @param s2 pointer to second t_symbol to symb_id_map_compare
 * @return -1 if s1 < s2; 0 if s1 == s2; 1 if s1 > s2
 */
int symb_cmp(t_symbol *s1, t_symbol *s2) {
    int res = 0;
    if(s1 > s2) {
        res = 1;
    } else if(s1 < s2) {
        res = -1;
    }
    return res;
}

/**
 * Comparator function for map implementation
 */
int symb_id_map_compar(const void *l, const void *r) {
    const symb_id_map *lm = l;
    const symb_id_map *lr = r;
    return symb_cmp(lm->key, lr->key);
}

/**
 * Adds symbol-ID pair to map
 *
 * @param s pointer to symbol
 * @param id ID value
 */
void add_symb_id_pair(t_symbol *s, int id) {
    symb_id_map *pair= malloc(sizeof(symb_id_map));
    pair->key = s;
    pair->value = id;
    tsearch(pair, &symb_id_map_root, symb_id_map_compar);
}

/**
 * Initializes symbol-ID map with symbol-ID pairs for 2 phrases, 2 channels and 4 tracks
 */
void init_symb_id_map(void) {
    symb_id_map_root = 0;
    symb_id_map_find = malloc(sizeof(symb_id_map));

    add_symb_id_pair(gensym("p1"), 0);
    add_symb_id_pair(gensym("p2"), 1);
    add_symb_id_pair(gensym("ch1"), 0);
    add_symb_id_pair(gensym("ch2"), 1);
    add_symb_id_pair(gensym("t1"), 0);
    add_symb_id_pair(gensym("t2"), 1);
    add_symb_id_pair(gensym("t3"), 2);
    add_symb_id_pair(gensym("t4"), 3);
    add_symb_id_pair(gensym("a"), 0);
    add_symb_id_pair(gensym("b"), 1);
}

/**
 * Retrieves an ID for the symbol
 *
 * @param s pointer to symbol for which ID will be retrieved
 * @return ID for given symbol
 */
int get_id_for_symb(t_symbol *s) {
    symb_id_map_find->key = s;
    void *r = tfind(symb_id_map_find, &symb_id_map_root, symb_id_map_compar);
    return (*(symb_id_map**)r)->value;
}


