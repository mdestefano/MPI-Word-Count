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

int wc_sum_array(int const *array, int array_size){
	int sum = 0;
	for(int i = 0; i<array_size;i++){
		sum += array[i];
	}	
	return sum;
}

char * string_to_lowercase(char *string){
	char *p = calloc(strlen(string)+1,sizeof(char));
	for(int i = 0; string[i]; i++){
		p[i] = tolower(string[i]);
	}
	return p;
}


int checkString( const char s[] ){
    unsigned char c;

    while ( ( c = *s ) && ( isalpha( c ))  ) ++s;

    return *s == '\0'; 
}