#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "mpi.h"
#define WORD_SIZE 64

MPI_Datatype mpi_woccurence_type;

typedef struct woccurrence{
    char word[WORD_SIZE];
    int count;
}woccurrence;

void initialize_woccurence_type();

woccurrence new_woccurence(const char *word);

void add_occurrence(woccurrence *wordocc);

void add_n_occurrence(woccurrence *wordocc, int occurrences);

void print_occurrence(woccurrence occ);
