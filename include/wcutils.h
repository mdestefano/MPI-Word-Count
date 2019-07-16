#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

char** wc_init_string_array(char **array,int size, int string_size);

int* wc_init_int_array(int* array, int size);

int wc_sum_array(int const *array, int array_size);

char * string_to_lowercase(char *string);

int checkString( const char s[] );