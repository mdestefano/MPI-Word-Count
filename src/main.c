#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "chunk.h"
#include "woccurence.h"
#include "wordsmap.h"

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

chunk* prepare_chunks(char **filenames, int *file_lines_count, size_t n_of_files,int nofproc,chunk *output_chunks, size_t *nofchunks, int *chunk_sendcount, int* chunk_dspls){
	int total_lines = 0, lines_per_processor, reminder;
	int lines_left, current_file_lines_left;
	size_t file_index = 0, old_file_index = 0;
	size_t chunks_size = nofproc, actual_chunks_size = 0;
	int line_index = 0;
	size_t start_index = 0,end_index = 0;
	int sum = 0;

	for(size_t i = 0;i<n_of_files;i++){
		total_lines += file_lines_count[i];
	}

	lines_per_processor = total_lines/nofproc;
	reminder = total_lines%nofproc;
	current_file_lines_left = file_lines_count[file_index];

	output_chunks = malloc(sizeof(chunk)*chunks_size);

	for(int current_proc = 0;current_proc<nofproc;current_proc++){
		lines_left = lines_per_processor;

		if(reminder > 0){
			lines_left += 1;
			reminder--;
		}
		printf("DBG: proc %d should have %d lines\n",current_proc,lines_left);

		while (lines_left>0){			
			
			old_file_index = file_index;

			if(lines_left>=current_file_lines_left){
				//crea nuovo chunk
				start_index = line_index;
				end_index = file_lines_count[file_index] - 1;
				
				lines_left -= (file_lines_count[file_index] -line_index);
				line_index = 0;				
				file_index += 1;
				current_file_lines_left = file_lines_count[file_index];				
			} else {
				//crea nuovo chunk
				
				start_index = line_index;
				end_index = line_index + lines_left-1;				
				line_index += lines_left;
				current_file_lines_left -= lines_left;
				lines_left = 0;
			}

			printf("DBG: created new chunk in file %s from %zu to %zu for proc %d \n",filenames[old_file_index],start_index,end_index,current_proc);
			

			if(actual_chunks_size>chunks_size){
				chunks_size *= 2;
				output_chunks = realloc(output_chunks,chunks_size*sizeof(chunk));	
				printf("DBG: chunk resize\n");				
			}

			output_chunks[actual_chunks_size] = new_chunk(filenames[old_file_index],start_index,end_index);
			actual_chunks_size++;
			chunk_sendcount[current_proc] += 1;
			

		}
		chunk_dspls[current_proc] = sum;
		sum += chunk_sendcount[current_proc];
		
		
	}

	(*nofchunks) = actual_chunks_size;
	return output_chunks;

}

wordsmap count_words_in_chunks(chunk* chunks, int nofchunks,wordsmap map){
	FILE* curr_file;
	size_t curr_line_index,len;
	ssize_t linesize;
	map = new_wordsmap();
	char* curr_line = NULL;

	for(int curr_chunk = 0; curr_chunk<nofchunks;curr_chunk++){
		print_chunk(chunks[curr_chunk]);
		curr_file = fopen(get_chunk_filename(chunks[curr_chunk]),"r");
		if(curr_file == NULL){
			printf("Error opening files\n");
			exit(EXIT_FAILURE);
		}

		curr_line_index = 0;
		while(curr_line_index < get_chunk_start_index(chunks[curr_chunk])){
			getline(&curr_line,&len,curr_file);
			curr_line_index++;		
			free(curr_line);
			curr_line = NULL;	
		}

		do{
			linesize = getline(&curr_line,&len,curr_file);
			if(linesize != -1){
				curr_line[strlen(curr_line)-1] = '\0';
				printf("DBG: curr_line_index = %zu Processing line: %s\n",curr_line_index,curr_line);
				
				free(curr_line);
				curr_line = NULL;
			}
			curr_line_index++;
		} while(curr_line_index <= get_chunk_end_index(chunks[curr_chunk]) && linesize != -1);

	}


	return map;
}


int main(int argc, char *argv[]) {
	
	int my_rank,nofproc, local_file_count;
	int *file_sendcounts, *file_dspls,*local_file_lines_count = NULL, *global_file_lines_count;
	size_t n_of_files,nofchunks;
	char **filenames = NULL, **local_filenames;
	int send_index;
	chunk *chunk_collection = NULL,*local_chunks = NULL;
	int *chunk_sendcount,*chunk_dspls;
	int local_chunk_count;
	wordsmap map = NULL;

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
			printf("DBG: Master keeps %s\n",local_filenames[i]);
		}

	} else {
		for(int i= 0; i<local_file_count;i++){
			MPI_Recv(local_filenames[i],FILENAME_SIZE,MPI_CHAR,MASTER,MPI_ANY_TAG,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
			printf("DBG: Process %d recieved %s\n",my_rank,local_filenames[i]);
		}
	} 

	local_file_lines_count = malloc(local_file_count*sizeof(int));
	for(int i = 0; i<local_file_count;i++){
		local_file_lines_count[i] = 0;
	}
	count_file_lines(local_filenames,local_file_lines_count,local_file_count);
	MPI_Gatherv(local_file_lines_count,local_file_count,MPI_INT,global_file_lines_count,file_sendcounts,file_dspls,MPI_INT,MASTER,MPI_COMM_WORLD);

	if(my_rank == MASTER){
		chunk_sendcount = malloc(nofproc*sizeof(int));
		chunk_dspls = malloc(nofproc*sizeof(int));
		if(chunk_sendcount == NULL || chunk_dspls == NULL){
			printf("Memory Error!\n");
			exit(EXIT_FAILURE);
		}
		for(int i = 0;i<nofproc;i++){
			chunk_sendcount[i] = 0;
			chunk_dspls[i] = 0;
		}
		chunk_collection = prepare_chunks(filenames,global_file_lines_count,n_of_files,nofproc,chunk_collection,&nofchunks,chunk_sendcount,chunk_dspls);		
	}

	//comune
	MPI_Scatter(chunk_sendcount,1,MPI_INT,&local_chunk_count,1,MPI_INT,MASTER,MPI_COMM_WORLD);	
	local_chunks = malloc(local_chunk_count*sizeof(chunk));
	
	if(local_chunks == NULL){
		printf("Memory erorr !\n");
		exit(EXIT_FAILURE);
	}

	for(int i = 0;i<local_chunk_count;i++){
		local_chunks[i] = new_empty_chunk();		
	}

	/* if chunk structure is change not to be used as a pointer to structure,
	 scatterv could be possible.*/
	if(my_rank == MASTER){
		send_index = chunk_sendcount[0];
		for(int reciever = 1; reciever <nofproc;reciever++){
			for(int i = 0;i<chunk_sendcount[reciever];i++){				
				MPI_Send(chunk_collection[send_index],1,mpi_text_file_chunk,reciever,MASTER,MPI_COMM_WORLD);
				send_index++;
			}
		}

		for(int i = 0;i<local_chunk_count;i++){
			local_chunks[i] = chunk_collection[i];
		}

	} else {
		for(int i= 0; i<local_chunk_count;i++){
			MPI_Recv(local_chunks[i],1,mpi_text_file_chunk,MASTER,MPI_ANY_TAG,MPI_COMM_WORLD,MPI_STATUS_IGNORE);			
		}
	}
	
	map = count_words_in_chunks(local_chunks,local_chunk_count,map);
	
	//gather dei dizionari

	//---master
	//fondi i dizionari


	MPI_Finalize();
}