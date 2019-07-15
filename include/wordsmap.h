#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include "woccurence.h"

typedef struct wordsmap {
    int size;
    int real_size;
    woccurrence* occurrences;
}wordsmap;

wordsmap new_wordsmap();

void add_word(wordsmap *map,const char word[]);

woccurrence* get_woccurrences_collection(wordsmap map,int *size);

void print_map(wordsmap map);

/*wordsmap merge_wordoccuurences(woccurrence **occurrences_collection,int *occurrences_count,int nofcollections);*/