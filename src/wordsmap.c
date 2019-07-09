#include "../include/wordsmap.h"
#include <stdio.h>

typedef struct wordsmap_s {
    int size;
    int real_size;
    woccurrence* occurrences;
}wordsmap_s;

wordsmap new_wordsmap(){
    wordsmap map = calloc(1,sizeof(wordsmap_s*));
    map->size = 256;
    map->real_size = 0;
    map->occurrences = calloc(map->size,sizeof(woccurrence));
    return map;
}

size_t hash(const char *str) {
    size_t hash = 5381;
    int c,i = 0;

    while ((c = str[i])) {
        hash =  hash * 33 + c;/*((hash << 5) + hash) + c;  hash * 33 + c */
        i++;
    }
    return hash;
}

void add_word(wordsmap map,const char word[]){
    
    bool found = false, empty_bucket = false;
    int index = 0,counter = 0;    
    woccurrence cur_occurence;
    woccurrence* buf;

    if(map->real_size == map->size){        
        printf("DBG: realloc map!\n");
        printf("DBG: map_size = %d, map_realsize %d!\n",map->size,map->real_size);
        map->size = map->size * 2;
        
        //map->occurrences = realloc(map->occurrences,map->size);
        buf = calloc(map->size,sizeof(woccurrence));
        for(int i=0;i<map->real_size;i++){
            buf[i] = map->occurrences[i];
        }
        free(map->occurrences);
        map->occurrences = buf;
    }

    while(!found && !empty_bucket){
        index = (hash(word)+counter)%map->size;
        //printf("DBG: MAP: hash(%s) + %zu mod %zu = %zu\n",word,counter,map->size,index);
        cur_occurence = map->occurrences[index];
        empty_bucket = cur_occurence == NULL;
        if(!empty_bucket){
            found = (strcmp(word,get_word(cur_occurence)) == 0);
        } 
        counter += 1;
    }

    if(empty_bucket){
        //printf("DBG: MAP: empty bucket at %zu\n",index);
        map->occurrences[index] = new_woccurence(word);
        //printf("DBG: OCCURRENCE (%s,%zu)\n",get_word(map->occurrences[index]),get_occurrences(map->occurrences[index]));
        map->real_size += 1;
    } else if (found){        
        add_occurrence(map->occurrences[index]);
        //printf("DBG: MAP: occurence at %zu incremented\n",index);
        //printf("DBG: OCCURRENCE (%s,%zu)\n",get_word(map->occurrences[index]),get_occurrences(map->occurrences[index]));
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
    int counter = 0;
    woccurrence *real_occurrences = calloc(map->real_size,sizeof(woccurrence));
    for(int i =0; i<map->size;i++){
        if(map->occurrences[i] != NULL){
            real_occurrences[counter] = map->occurrences[i];
            counter++;
        }
    }
    return real_occurrences;
}

int get_woccurrences_size(wordsmap map){
    return map->real_size;
}

void print_map(wordsmap map){    
    printf("MAP: size %d, real_ size %d \n",map->size,map->real_size);
    for(int i =0; i<map->size;i++){
        if(map->occurrences[i] != NULL){
            print_occurrence(map->occurrences[i]);
        }
    }    
}

wordsmap merge_wordoccuurences(woccurrence **occurrences_collection,int *occurrences_count,int nofcollections){
    wordsmap map = new_wordsmap();
    int remaining_occ;
    for(int i = 0;i<nofcollections;i++){
        for(int j = 0; j<occurrences_count[i];j++){
            remaining_occ = get_occurrences(occurrences_collection[i][j]);
            while(remaining_occ>0){               
                add_word(map,get_word(occurrences_collection[i][j]));
                remaining_occ--;
            }
        }
    }
    return map;
}