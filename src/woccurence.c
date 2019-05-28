#include <stddef.h>
#include "../include/woccurence.h"

typedef struct woccurrence_c{
    char word[WORD_SIZE];
    size_t count;
}woccurence_c;

void initialize_woccurence_type(){
    MPI_Datatype types[2] = {MPI_CHAR, MPI_UNSIGNED};
  int blocklengths[2] = {WORD_SIZE,1};
  MPI_Aint offsets[2] = {
        offsetof(woccurence_c, word),
        offsetof(woccurence_c, count),
  };

  MPI_Type_create_struct(2, blocklengths, offsets, types, &mpi_woccurence_type);
  MPI_Type_commit(&mpi_woccurence_type);
}



