#include "../include/woccurence.h"


void initialize_woccurence_type(){
    MPI_Datatype types[2] = {MPI_CHAR, MPI_INT};
  int blocklengths[2] = {WORD_SIZE,1};
  MPI_Aint offsets[2] = {
        offsetof(woccurrence, word),
        offsetof(woccurrence, count),
  };

  MPI_Type_create_struct(2, blocklengths, offsets, types, &mpi_woccurence_type);
  MPI_Type_commit(&mpi_woccurence_type);
}

woccurrence new_woccurence(const char *word){
  woccurrence woc;
  strcpy(woc.word,word);
  woc.count = 1;

  return woc;
}

void add_occurrence(woccurrence *wordocc){
  add_n_occurrence(wordocc,1);
}

void add_n_occurrence(woccurrence *wordocc, int occurrences){
  wordocc->count += occurrences;
}

void print_occurrence(woccurrence occ){
  printf("(%s,%d)\n",occ.word,occ.count);
}

