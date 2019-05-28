#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include "../include/chunk.h"

typedef struct chunk_c {
  char filename[FILENAME_SIZE]; // può essere migliorato con una allocazione dinamica
  size_t start_index;
  size_t end_index;
}chunk_c;

void initialize_chunk_type(){
  MPI_Datatype types[3] = {MPI_CHAR, MPI_UNSIGNED, MPI_UNSIGNED};
  int blocklengths[3] = {FILENAME_SIZE,1,1};
  MPI_Aint offsets[3] = {
        offsetof(chunk_c, filename),
        offsetof(chunk_c, start_index),
        offsetof(chunk_c, end_index),
  };

  MPI_Type_create_struct(3, blocklengths, offsets, types, &mpi_text_file_chunk);
  MPI_Type_commit(&mpi_text_file_chunk);
}

chunk new_chunk(const char filename[], size_t start_index,size_t end_index){
    struct chunk_c *a_chunk = malloc(sizeof(struct chunk_c));
    if(a_chunk == NULL){
        return NULL;
    }
    //a_chunk->filename = malloc(strlen(filename)+1);
    if(a_chunk->filename == NULL){
        return NULL;
    }
    strcpy(a_chunk->filename,filename);
    a_chunk->start_index = start_index;
    a_chunk->end_index = end_index;
    return a_chunk;
}

char* get_chunk_filename(chunk c){
    return c->filename;
}

size_t get_chunk_start_index(chunk c){
    return c->start_index;
}

size_t get_chunk_end_index(chunk c){
    return c->end_index;
}

bool chunk_equals(chunk c1,chunk c2){
    if(strcmp(c1->filename,c2->filename) != 0){
        return false;
    }

    if(c1->start_index != c2->end_index){
        return false;
    }

    if(c1->start_index != c2->end_index){
        return false;
    }

    return true;
}
