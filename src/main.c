#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "chunk.h"
#include "woccurence.h"

#define MASTER 0

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
	fclose(index_file);
	(*n) = i;
	return filenames;
}

void divide_files(size_t n_of_files,int nofproc,int *sendcounts, int *displs){	
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
}

void count_file_lines(char **filenames,int *files_line_count,int n_of_files){
	char* line = NULL;
	size_t len;
	for(int i = 0;i<n_of_files;i++){		
		FILE* current_file = fopen(filenames[i],"r");
		if(filenames == NULL){
			printf("Error opening files");
			exit(EXIT_FAILURE);
		}
		while (getline(&line,&len,current_file) != -1){
			files_line_count[i] += 1;
		}		
		fclose(current_file);
	}
}

void prepare_chunks(char **filenames, int *file_lines_count, size_t n_of_files,int nofproc,chunk *output_chunks, size_t *nofchunks, int *chunk_sendcount, int* chunk_dspls){
	int total_lines = 0, lines_per_processor, reminder;
	int lines_left, current_file_lines_left;
	size_t file_index = 0;
	int line_index = 0;

	for(size_t i = 0;i<n_of_files;i++){
		total_lines += file_lines_count[i];
	}

	lines_per_processor = total_lines/nofproc;
	reminder = total_lines%nofproc;
	current_file_lines_left = file_lines_count[file_index];

	//output_chunks = malloc(sizeof(chunk)*(*nofchunks));

	for(int current_proc = 0;current_proc<nofproc;current_proc++){
		lines_left = lines_per_processor;

		if(reminder > 0){
			lines_left += 1;
			reminder--;
		}
		printf("DBG: proc %d should have %d lines\n",current_proc,lines_left);

		while (lines_left>0){			
			
			if(lines_left>=current_file_lines_left){
				//crea nuovo chunk
				printf("DBG: created new chunk in file %s from %d to %d for proc %d \n",filenames[file_index],line_index,file_lines_count[file_index] - 1,current_proc);
				lines_left -= (file_lines_count[file_index] -line_index);
				line_index = 0;
				file_index += 1;
				current_file_lines_left = file_lines_count[file_index];
				printf("DBG lines left %d\n",lines_left);
			} else {
				//crea nuovo chunk
				printf("DBG: created new chunk in file %s from %d to %d for proc %d \n",filenames[file_index],line_index,line_index + lines_left-1,current_proc);
				line_index += lines_left;
				current_file_lines_left -= lines_left;
				lines_left = 0;
			}
			

			/*if(actual_chunks_size>(*nofchunks)){
				(*nofchunks) *= 2;
				output_chunks = realloc(output_chunks,(*nofchunks)*sizeof(chunk));					
			}

			output_chunks[actual_chunks_size] = new_chunk(filenames[file_index],)*/

		
		}
		
	}

}

int main(int argc, char *argv[]) {
	
	int my_rank,nofproc, local_file_count;
	int *file_sendcounts, *file_dspls,*local_file_lines_count = NULL, *global_file_lines_count;
	size_t n_of_files;
	char **filenames = NULL, **local_filenames;
	int send_index;

	MPI_Init(&argc,&argv);
	MPI_Comm_rank(MPI_COMM_WORLD,&my_rank);
	MPI_Comm_size(MPI_COMM_WORLD,&nofproc);
	init_custom_types();
	
	

	if(my_rank == MASTER){
		
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
		global_file_lines_count = malloc(n_of_files*sizeof(int));
		if(global_file_lines_count == NULL){
			printf("Memory error !");
			exit(EXIT_FAILURE);
		}
	}
	
	//--comune
	puts("DBG: COMUNICATION STARTING");
	MPI_Scatter(file_sendcounts,1,MPI_INT,&local_file_count,1,MPI_INT,MASTER,MPI_COMM_WORLD);
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
	if(my_rank == MASTER){
		for(int reciever = 1; reciever <nofproc;reciever++){
			for(int i = 0;i<file_sendcounts[reciever];i++){
				printf("Master sending %s\n",filenames[send_index]);
				MPI_Send(filenames[send_index],strlen(filenames[send_index])+1,MPI_CHAR,reciever,MASTER,MPI_COMM_WORLD);
				send_index++;
			}
		}

		for(int i = 0;i<local_file_count;i++){
			local_filenames[i] = strcpy(local_filenames[i],filenames[i]);
			printf("Master keeps %s\n",local_filenames[i]);
		}

	} else {
		for(int i= 0; i<local_file_count;i++){
			MPI_Recv(local_filenames[i],FILENAME_SIZE,MPI_CHAR,MASTER,MPI_ANY_TAG,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
			printf("process %d recieved %s\n",my_rank,local_filenames[i]);
		}
	} 

	local_file_lines_count = malloc(local_file_count*sizeof(int));
	for(int i = 0; i<local_file_count;i++){
		local_file_lines_count[i] = 0;
	}
	count_file_lines(local_filenames,local_file_lines_count,local_file_count);
	MPI_Gatherv(local_file_lines_count,local_file_count,MPI_INT,global_file_lines_count,file_sendcounts,file_dspls,MPI_INT,MASTER,MPI_COMM_WORLD);

	if(my_rank == MASTER){
		for(size_t i = 0;i<n_of_files;i++){
			printf("MASTER: DBG: file %s has %d lines\n",filenames[i],global_file_lines_count[i]);
		}
	}

	if(my_rank == MASTER){
		prepare_chunks(filenames,global_file_lines_count,n_of_files,nofproc,NULL,NULL,NULL,NULL);
	}
	//prepara i chunk

	//comune
	//scatter(v) dei chunk
	//per ogni chunk, conta il numero di parole nel chunk e mettile nel dizionario locale
	//gather dei dizionari

	//---master
	//fondi i dizionari


	MPI_Finalize();
}