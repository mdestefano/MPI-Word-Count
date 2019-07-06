#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "chunk.h"
#include "woccurence.h"

void init_custom_types(){
	initialize_chunk_type();
	initialize_woccurence_type();
}

char** detect_files(char **filenames, size_t *n){
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
	while (getline(&filenames[i],&len,index_file) != -1){
		filenames[i][strlen(filenames[i])-1] = '\0';	
		i++;
		if(i>=current_list_size){		
			current_list_size = current_list_size *2;
			filenames = realloc(filenames,current_list_size*sizeof(char*));

			for(size_t j = i;j<current_list_size;j++){
				filenames[j] = NULL;
			}
		}
	}
	puts(filenames[2]);
	fclose(index_file);
	(*n) = i;
	return filenames;
}

void divide_files(size_t n_of_files,int nofproc,int *sendcounts, int *displs){	
	puts("DBG: MASTER: DIVIDE FILES: EVERITYHING OK SO FAR");
	int file_per_processor = n_of_files/nofproc;
	int rem = n_of_files%nofproc;
	int sum = 0;
	for(int i = 0; i<nofproc; i++){
		sendcounts[i] = file_per_processor;
		if(rem>0){
			sendcounts[i] += 1;
			rem--;
		}
		displs[i] = sum;
		sum += sendcounts[i];
	}
	for(int i = 0; i<nofproc;i++){
		printf("DIVIDE FILES: sendcounts[%d] = %d\n",i,sendcounts[i]);
	}
}

void count_file_lines(char **filenames,int *files_line_count,int n_of_files){
	char* line = NULL;
	size_t len;
	for(int i = 0;i<n_of_files;i++){
		printf("DBG: opening %s\n",filenames[i]);
		FILE* current_file = fopen(filenames[i],"r");
		if(filenames == NULL){
			printf("Error opening files");
			exit(EXIT_FAILURE);
		}
		while (getline(&line,&len,current_file) != -1){
			files_line_count[i] += 1;
		}
		printf("DBG file %s has %d lines\n",filenames[i],files_line_count[i]);
		fclose(current_file);
	}
}

int main(int argc, char *argv[]) {
	
	int my_rank,nofproc, *file_sendcounts, *file_dspls, local_file_count,*local_file_lines_count = NULL;
	size_t n_of_files;
	char **filenames = NULL, **local_filenames;
	int send_index;

	MPI_Init(&argc,&argv);
	MPI_Comm_rank(MPI_COMM_WORLD,&my_rank);
	MPI_Comm_size(MPI_COMM_WORLD,&nofproc);
	init_custom_types();
	
	

	if(my_rank == 0){
		
		filenames = detect_files(filenames,&n_of_files);
		puts(filenames[0]);	
		file_sendcounts = malloc(nofproc*sizeof(int));
		file_dspls = malloc(nofproc*sizeof(int));

		if(file_sendcounts == NULL || file_dspls == NULL){
			printf("Memory error !");
			exit(EXIT_FAILURE);
		}		
		divide_files(n_of_files,nofproc,file_sendcounts,file_dspls);
		send_index = file_sendcounts[0];
	}
	
	//--comune
	puts("DBG: COMUNICATION STARTING");
	MPI_Scatter(file_sendcounts,1,MPI_INT,&local_file_count,1,MPI_INT,0,MPI_COMM_WORLD);
	printf("DBG: proc. %d recv_count %d\n",my_rank,local_file_count);

	local_filenames = malloc(local_file_count*sizeof(char*));	
	if(local_filenames == NULL){
		exit(EXIT_FAILURE);
	}

	for(int i = 0; i<local_file_count;i++){
		local_filenames[i] = malloc(FILENAME_SIZE * sizeof(char));	
		if(local_filenames[i] == NULL){
			exit(EXIT_FAILURE);
		}
	}
		
	puts("DBG: FILENAMES ARE ARRIVING!");
	if(my_rank == 0){
		for(int reciever = 1; reciever <nofproc;reciever++){
			for(int i = 0;i<file_sendcounts[reciever];i++){
				printf("Master sending %s\n",filenames[send_index]);
				MPI_Send(filenames[send_index],strlen(filenames[send_index])+1,MPI_CHAR,reciever,0,MPI_COMM_WORLD);
				send_index++;
			}
		}

		for(int i = 0;i<local_file_count;i++){
			local_filenames[i] = strcpy(local_filenames[i],filenames[i]);
			printf("Master keeps %s\n",local_filenames[i]);
		}

	} else {
		for(int i= 0; i<local_file_count;i++){
			MPI_Recv(local_filenames[i],FILENAME_SIZE,MPI_CHAR,0,MPI_ANY_TAG,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
			printf("process %d recieved %s\n",my_rank,local_filenames[i]);
		}
	} 

	local_file_lines_count = malloc(local_file_count*sizeof(int));
	for(int i = 0; i<local_file_count;i++){
		local_file_lines_count[i] = 0;
	}
	count_file_lines(local_filenames,local_file_lines_count,local_file_count);
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