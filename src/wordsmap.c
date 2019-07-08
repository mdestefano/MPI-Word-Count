#include "../include/wordsmap.h"
#include <stdio.h>

typedef struct wordsmap_s {
    size_t size;
    size_t real_size;
    woccurrence* occurrences;
}wordsmap_s;

wordsmap new_wordsmap(){
    wordsmap map = calloc(1,sizeof(wordsmap_s*));
    map->size = 64;
    map->real_size = 0;
    map->occurrences = calloc(map->size,sizeof(woccurrence));
    return map;
}

size_t hash(const char *str) {
    size_t hash = 5381;
    int c;

    while ((c = *str++))
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash;
}

void add_word(wordsmap map,char word[]){
    printf("DBG: MAP: adding %s\n",word);
    bool found = false, empty_bucket = false;
    size_t index = 0,counter = 0;    
    woccurrence cur_occurence;

    while(!found && !empty_bucket && index<map->size){
        index = (hash(word)+counter)%map->size;
        printf("DBG: MAP: hash(%s) + %zu mod %zu = %zu\n",word,counter,map->size,index);
        cur_occurence = map->occurrences[index];
        empty_bucket = cur_occurence == NULL;
        if(!empty_bucket){
            found = (strcat(word,get_word(cur_occurence)) == 0);
        } 
        counter += 1;
    }

    if(empty_bucket){
        printf("DBG: MAP: empty bucket at %zu\n",index);
        map->occurrences[index] = new_woccurence(word);
        printf("DBG: OCCURRENCE (%s,%zu)\n",get_word(map->occurrences[index]),get_occurrences(map->occurrences[index]));
        map->real_size += 1;
    } else if (found){        
        add_occurrence(map->occurrences[index]);
        printf("DBG: MAP: occurence at %zu incremented\n",index);
        printf("DBG: OCCURRENCE (%s,%zu)\n",get_word(map->occurrences[index]),get_occurrences(map->occurrences[index]));
    } else {
        map->size *= 2;
        map->occurrences = realloc(map->occurrences,map->size);
        printf("DBG: MAP: map resized\n");
    }
    


    /*int i;
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
    }*/    
}

woccurrence* get_word_occurrences(wordsmap map){
    return map->occurrences;
}

int get_woccurrences_size(wordsmap map){
    return map->real_size;
}

void print_map(wordsmap map){    
    printf("MAP: size %zu, real_ size %zu \n",map->size,map->real_size);
    for(size_t i =0; i<map->size;i++){
        if(map->occurrences[i] != NULL){
            printf("MAP: (%s,%zu)\n",get_word(map->occurrences[i]),get_occurrences(map->occurrences[i]));
        }
    }    
}