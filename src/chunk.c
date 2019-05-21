#include <stdlib.h>
#include <string.h>
#include "../include/chunk.h"



chunk new_chunk(const char filename[], size_t start_index,size_t end_index){
    struct chunk_c *a_chunk = malloc(sizeof(struct chunk_c));
    if(a_chunk == NULL){
        return NULL;
    }
    a_chunk->filename = malloc(strlen(filename)+1);
    if(a_chunk->filename == NULL){
        return NULL;
    }
    a_chunk->filename = strcpy(a_chunk->filename,filename);
    a_chunk->start_index = start_index;
    a_chunk->end_index = end_index;
    return a_chunk;
}
