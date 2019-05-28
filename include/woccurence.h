#include <stddef.h>
#include "mpi.h"
#include "../include/uthash.h"
#define WORD_SIZE 64

MPI_Datatype mpi_woccurence_type;

typedef struct woccurrence_c* woccurrence;

//uso la lista implementata con l'array in modo che posso poi ottenere oggetti già allocati contiguamente