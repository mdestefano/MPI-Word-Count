#include <stdio.h>
#include <stdlib.h>

char** wc_init_string_array(char **array,int size, int string_size);

int* wc_init_int_array(int* array, int size);

int wc_compute_total_lines(int const *lines_array, int array_size);