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

void add_n_word(wordsmap *map,char word[],int nofoccur);

void add_word(wordsmap *map,char word[]);

woccurrence* get_woccurrences_collection(wordsmap map,int *size);

void print_map(wordsmap map);

wordsmap merge_woccurrences(woccurrence *occurrences, int size);