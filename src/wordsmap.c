#include "../include/wordsmap.h"
#define EMPTY -1

wordsmap new_wordsmap(){
    wordsmap map;
    map.size = 256;
    map.real_size = 0;
    map.occurrences = calloc(map.size,sizeof(woccurrence));
    for(int i = 0; i<map.size;i++){
        map.occurrences[i].count = EMPTY;
    }
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

static void resize_map(wordsmap *map){    
    woccurrence* buf;    
    map->size = map->size * 2;
        
    //map->occurrences = realloc(map->occurrences,map->size);
    buf = calloc(map->size,sizeof(woccurrence));
    for(int i=0;i<map->real_size;i++){
        buf[i] = map->occurrences[i];
    }
    for(int i=map->real_size;i<map->size;i++){
        buf[i].count = EMPTY;
    }
    free(map->occurrences);
    map->occurrences = buf;
}

static bool is_empty_bucket(wordsmap map, int index){
    return map.occurrences[index].count == EMPTY;
}

void add_n_word(wordsmap *map, char word[],int nofoccur){    
    bool found = false, empty_bucket = false;
    int index = 0,counter = 0;    
    woccurrence cur_occurence;
    

    if(map->real_size >= map->size){        
       //printf("DBG: realloc map!\n");
       resize_map(map);
    }

    while(!found && !empty_bucket){
        index = (hash(word)+counter)%map->size;
        //printf("DBG: MAP: hash(%s) + %zu mod %zu = %zu\n",word,counter,map->size,index);
        cur_occurence = map->occurrences[index];
        empty_bucket = is_empty_bucket((*map),index);
        if(!empty_bucket){
            found = (strcmp(word,cur_occurence.word) == 0);
        } 
        counter += 1;
    }

    if(empty_bucket){
        //printf("DBG: MAP: empty bucket at %d\n",index);
        map->occurrences[index] = new_woccurence(word);  
        add_n_occurrence(&map->occurrences[index],nofoccur-1);      
        map->real_size += 1;
    } else if (found){        
        add_n_occurrence(&map->occurrences[index],nofoccur);
    }
}

void add_word(wordsmap *map,char word[]){    
    add_n_word(map,word,1);
}

woccurrence* get_woccurrences_collection(wordsmap map,int *size){
    int counter = 0;
    woccurrence *real_occurrences = calloc(map.real_size,sizeof(woccurrence));
    for(int i =0; i<map.size;i++){
        if(!is_empty_bucket(map,i)){
            real_occurrences[counter] = map.occurrences[i];
            counter++;
        }
    }
    (*size) =  map.real_size;
    return real_occurrences;
}

void print_map(wordsmap map){    
    printf("MAP: size %d, real_ size %d \n",map.size,map.real_size);
    for(int i =0; i<map.size;i++){
        if(!is_empty_bucket(map,i)){
            print_occurrence(map.occurrences[i]);
        }
    }    
}

wordsmap merge_woccurrences(woccurrence *occurrences, int size){
    wordsmap map = new_wordsmap();

    for (int i = 0; i < size; i++) {
        add_n_word(&map,occurrences[i].word,occurrences[i].count);
    }
    return map;
}
