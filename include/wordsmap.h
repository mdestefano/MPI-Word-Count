#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "woccurence.h"

typedef struct wordsmap_s* wordsmap;

wordsmap new_wordsmap();

void add_word(wordsmap map,char const word[]);

woccurrence* get_word_occurrences(wordsmap map);

int get_woccurrences_size(wordsmap map);