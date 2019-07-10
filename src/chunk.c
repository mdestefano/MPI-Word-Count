#include "../include/chunk.h"


void initialize_chunk_type(){
  MPI_Datatype types[3] = {MPI_CHAR, MPI_INT, MPI_INT};
  int blocklengths[3] = {FILENAME_SIZE,1,1};
  MPI_Aint offsets[3] = {
        offsetof(chunk, filename),
        offsetof(chunk, start_index),
        offsetof(chunk, end_index),
  };

  MPI_Type_create_struct(3, blocklengths, offsets, types, &mpi_text_file_chunk);
  MPI_Type_commit(&mpi_text_file_chunk);
}

chunk new_chunk(const char filename[], int start_index, int end_index){
    chunk a_chunk;
    a_chunk.start_index=start_index;
    a_chunk.end_index=end_index;
    strcpy(a_chunk.filename,filename);    
    return a_chunk;
}

bool chunk_equals(chunk c1,chunk c2){
    if(strcmp(c1.filename,c2.filename) != 0){
        return false;
    }

    if(c1.start_index != c2.end_index){
        return false;
    }

    if(c1.start_index != c2.end_index){
        return false;
    }

    return true;
}

void print_chunk(chunk c1){
    printf("CHUNK: filename=%s, start_index=%d, end_index=%d\n",c1.filename,c1.start_index,c1.end_index);
}