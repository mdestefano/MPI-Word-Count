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

int main(int argc, char *argv[]) {
	
	int my_rank,n_of_processors, *file_sendcounts, *file_dspls, recv_file_count;
	size_t n_of_files;
	char **filenames = NULL, **recv_file_buf;
	int send_index;

	MPI_Init(&argc,&argv);
	MPI_Comm_rank(MPI_COMM_WORLD,&my_rank);
	MPI_Comm_size(MPI_COMM_WORLD,&n_of_processors);
	init_custom_types();
	
	

	if(my_rank == 0){
		
		filenames = detect_files(filenames,&n_of_files);
		puts(filenames[0]);	
		file_sendcounts = malloc(n_of_processors*sizeof(int));
		file_dspls = malloc(n_of_processors*sizeof(int));

		if(file_sendcounts == NULL || file_dspls == NULL){
			printf("Memory error !");
			exit(EXIT_FAILURE);
		}		
		divide_files(n_of_files,n_of_processors,file_sendcounts,file_dspls);
		send_index = file_sendcounts[0];
	}
	
	//--comune
	puts("DBG: COMUNICATION STARTING");
	MPI_Scatter(file_sendcounts,1,MPI_INT,&recv_file_count,1,MPI_INT,0,MPI_COMM_WORLD);
	printf("DBG: proc. %d recv_count %d\n",my_rank,recv_file_count);

	recv_file_buf = malloc(recv_file_count*sizeof(char*));	
	if(recv_file_buf == NULL){
		exit(EXIT_FAILURE);
	}

	for(int i = 0; i<recv_file_count;i++){
		recv_file_buf[i] = malloc(FILENAME_SIZE * sizeof(char));	
		if(recv_file_buf[i] == NULL){
			exit(EXIT_FAILURE);
		}
	}
		
	puts("DBG: FILENAMES ARE ARRIVING!");
	if(my_rank == 0){
		for(int reciever = 1; reciever <n_of_processors;reciever++){
			for(int i = 0;i<file_sendcounts[reciever];i++){
				printf("Master sending %s\n",filenames[send_index]);
				MPI_Send(filenames[send_index],strlen(filenames[send_index])+1,MPI_CHAR,reciever,0,MPI_COMM_WORLD);
				send_index++;
			}
		}
	} else {
		for(int i= 0; i<recv_file_count;i++){
			MPI_Recv(recv_file_buf[i],FILENAME_SIZE,MPI_CHAR,0,MPI_ANY_TAG,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
			printf("process %d recieved %s\n",my_rank,recv_file_buf[i]);
		}
	} 
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