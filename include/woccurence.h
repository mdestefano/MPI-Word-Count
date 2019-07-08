#include <stdlib.h>
#include "mpi.h"
#define WORD_SIZE 64

MPI_Datatype mpi_woccurence_type;

typedef struct woccurrence_c* woccurrence;

void initialize_woccurence_type();

woccurrence new_woccurence(const char *word);

void add_occurrence(woccurrence wordocc);

void add_n_occurrence(woccurrence wordocc, int occurrences);

int get_occurrences(woccurrence wordocc);

char* get_word(woccurrence wordocc);

void print_occurrence(woccurrence occ);