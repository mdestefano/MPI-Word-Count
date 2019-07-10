#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include "mpi.h"
#define FILENAME_SIZE 128

MPI_Datatype mpi_text_file_chunk;

typedef struct chunk {
  char filename[FILENAME_SIZE]; // può essere migliorato con una allocazione dinamica
  size_t start_index;
  size_t end_index;
}chunk;

void initialize_chunk_type();

chunk new_chunk(const char filename[], size_t start_index,size_t end_index);

bool chunk_equals(chunk c1,chunk c2);

void print_chunk(chunk c1);

