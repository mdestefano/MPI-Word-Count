#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "chunk.h"
#include "woccurence.h"

void init_custom_types(){
	initialize_chunk_type();
	initialize_woccurence_type();
}

void detect_files(char **filenames, size_t *n){
	puts("DBG: MASTER: INSIDE DETECT FILE");
	size_t current_list_size = 2;
	size_t i = 0, len = 0;

	FILE* index_file = fopen("input/index.txt","r");
	filenames = malloc(current_list_size*sizeof(char*));
	if(filenames == NULL){
		printf("Memory Error !");
		exit(EXIT_FAILURE);
	}
	for(size_t i = 0;i<current_list_size;i++){
		filenames[i] = NULL;
	}
	puts("DBG: MASTER: LIST OF STRING ALLOCATED");
	while (getline(&filenames[i],&len,index_file) != -1){
		filenames[i][strlen(filenames[i])-1] = '\0';	
		printf("DBG: MASTER: ITERATION %zu, read %s\n",i,filenames[i]);	
		i++;
		if(i>=current_list_size){
			puts("SMALLER");
			current_list_size = current_list_size *2;
			filenames = realloc(filenames,current_list_size*sizeof(char*));

			for(size_t j = i;j<current_list_size;j++){
				filenames[j] = NULL;
			}
		}
	}
	for(size_t j = 0;j<i;j++){
		puts(filenames[j]);
	}

	fclose(index_file);
	(*n) = i;
}

void divide_files(size_t n_of_files,int nofproc,int *sendcounts, int *displs){	
	int file_per_processor = n_of_files/nofproc;
	int rem = n_of_files%nofproc;
	for(int i = 0; i<rem; i++){
		sendcounts[i] = file_per_processor + 1;
		displs[i] = i +1;
	}

	for(int i = rem; i<nofproc;i++){
		sendcounts[i] = file_per_processor;
		displs[i] = i;
	}
}

int main(int argc, char *argv[]) {
	
	int my_rank,n_of_processors, *file_sendcounts, *file_delims;
	size_t n_of_files;

	MPI_Init(&argc,&argv);
	MPI_Comm_rank(MPI_COMM_WORLD,&my_rank);
	MPI_Comm_size(MPI_COMM_WORLD,&n_of_processors);
	init_custom_types();
	
	char **filenames = NULL;
	puts("DBG: EVERITYHING OK SO FAR");
	if(my_rank == 0){
		detect_files(filenames,&n_of_files);
		puts("DBG: MASTER: EVERITYHING OK SO FAR");
		file_sendcounts = malloc(n_of_processors*sizeof(int));
		file_delims = malloc(n_of_processors*sizeof(int));

		if(file_sendcounts == NULL || file_delims == NULL){
			printf("Memory error !");
			exit(EXIT_FAILURE);
		}		

	}
	
	//--comune	
	//distribuisci i files tra i processori (scatter)
	//conta il numero di righe
	//restituisci il numero di righe (gather)

	//---master 
	//prepara i chunk

	//comune
	//scatter(v) dei chunk
	//per ogni chunk, conta il numero di parole nel chunk e mettile nel dizionario locale
	//gather dei dizionari

	//---master
	//fondi i dizionari


	MPI_Finalize();
}