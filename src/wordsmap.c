#include "../include/wordsmap.h"


typedef struct wordsmap_s {
    int size;
    int real_size;
    woccurrence* occurrences;
}wordsmap_s;

wordsmap new_wordsmap(){
    wordsmap map = calloc(1,sizeof(wordsmap_s*));
    map->size = 64;
    map->real_size = 0;
    map->occurrences = calloc(map->size,sizeof(woccurrence));
    return map;
}

void add_word(wordsmap map,char const word[]){
    bool found = false;
    int i;
    for(i=0;i<map->real_size && !found;i++){
        found = strcmp(word,get_word(map->occurrences[i]));
    }

    if(found){
        add_occurrence(map->occurrences[i]);
    } else {
        
        if(map->real_size >= map->size){
            map->size *= 2;
            map->occurrences = realloc(map->occurrences,map->size);
        }

        map->real_size += 1;
        map->occurrences[map->real_size] = new_woccurence(word);
    }
}

woccurrence* get_word_occurrences(wordsmap map){
    return map->occurrences;
}

int get_woccurrences_size(wordsmap map){
    return map->real_size;
}