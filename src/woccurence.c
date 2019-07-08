#include <string.h>
#include <stdio.h>
#include "../include/woccurence.h"

typedef struct woccurrence_c{
    char word[WORD_SIZE];
    int count;
}woccurence_c;

void initialize_woccurence_type(){
    MPI_Datatype types[2] = {MPI_CHAR, MPI_INT};
  int blocklengths[2] = {WORD_SIZE,1};
  MPI_Aint offsets[2] = {
        offsetof(woccurence_c, word),
        offsetof(woccurence_c, count),
  };

  MPI_Type_create_struct(2, blocklengths, offsets, types, &mpi_woccurence_type);
  MPI_Type_commit(&mpi_woccurence_type);
}

woccurrence new_woccurence(const char *word){
  struct woccurrence_c *woc = malloc(sizeof(woccurence_c));
  if(woc == NULL){
    return NULL;
  }

  strcpy(woc->word,word);
  woc->count = 1;

  return woc;
}

void add_occurrence(woccurrence wordocc){
  add_n_occurrence(wordocc,1);
}

void add_n_occurrence(woccurrence wordocc, int occurrences){
  wordocc->count += occurrences;
}

int get_occurrences(woccurrence wordocc){
  return wordocc->count;
}

char* get_word(woccurrence wordocc){
  return wordocc->word;
}

void print_occurrence(woccurrence occ){
  printf("(%s,%d)\n",occ->word,occ->count);
}

