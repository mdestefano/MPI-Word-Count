#include<stdbool.h>
#include<stddef.h>
#include "mpi.h"
#define FILENAME_SIZE 128

MPI_Datatype mpi_text_file_chunk;

typedef struct chunk_c* chunk;

void initialize_chunk_type();

chunk new_chunk(const char filename[], size_t start_index,size_t end_index);

char* get_chunk_filename(chunk c);

size_t get_chunk_start_index(chunk c);

size_t get_chunk_end_index(chunk c);

bool chunk_equals(chunk c1,chunk c2);

void print_chunk(chunk c1);

chunk new_empty_chunk();
