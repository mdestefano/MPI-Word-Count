#include "wcutils.h"

char** wc_init_string_array(char **array,int size, int string_size){
	if(size < 0 || string_size < 0 ){
		printf("Sizes can't be negative\n");
		return NULL;
	}

	array = calloc(size,sizeof(char*));	
	if(array == NULL){
		exit(EXIT_FAILURE);
	}

	for(int i = 0; i<size;i++){
		array[i] = calloc(string_size,sizeof(char));	
		if(array[i] == NULL){
			exit(EXIT_FAILURE);
		}
	}

	return array;

}

int* wc_init_int_array(int* array, int size){
	if(size < 0){
		printf("Size can't be negative\n");
		return NULL;
	}

	array = calloc(size,sizeof(int*));	
	if(array == NULL){
		exit(EXIT_FAILURE);
	}	
	
	return array;
}

int wc_compute_total_lines(int const *lines_array, int array_size){
	int sum = 0;
	for(int i = 0; i<array_size;i++){
		sum += lines_array[i];
	}	
	return sum;
}