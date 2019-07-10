#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "chunk.h"

#define MASTER 0

void init_custom_types(){
	initialize_chunk_type();
	//initialize_woccurence_type();
}

char** detect_files(char **filenames, size_t *n,char *index_filename){
	size_t current_list_size = 16;
	size_t i = 0, len = 0;
	ssize_t line_size;

	FILE* index_file = fopen(index_filename,"r");
	filenames = calloc(current_list_size,sizeof(char*));
	if(filenames == NULL){
		printf("Memory Error !");
		exit(EXIT_FAILURE);
	}
	while ((line_size = getline(&filenames[i],&len,index_file)) != -1){
		if(filenames[i][line_size-1] == '\n'){
			filenames[i][line_size-1] = '\0';
		}
		//printf("DBG: filename: %s\n",filenames[i]);
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
	//printf("DBG: COUNTING LINES\n");
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
			free(line);
			line = NULL;
		}		
		//printf("noflines %d\n",files_line_count[i]);
		fclose(current_file);
	}
}

chunk* prepare_chunks(char **filenames, int *file_lines_count, size_t n_of_files,int nofproc,chunk *output_chunks, size_t *nofchunks, int *chunk_sendcount, int* chunk_dspls,int *output_total_lines){
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

	*output_total_lines = total_lines;

	lines_per_processor = total_lines/nofproc;
	reminder = total_lines%nofproc;
	current_file_lines_left = file_lines_count[file_index];

	output_chunks = calloc(chunks_size,sizeof(chunk));

	for(int current_proc = 0;current_proc<nofproc;current_proc++){
		lines_left = lines_per_processor;

		if(reminder > 0){
			lines_left += 1;
			reminder--;
		}
		//printf("DBG: proc %d should have %d lines\n",current_proc,lines_left);

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

			//printf("DBG: created new chunk in file %s from %zu to %zu for proc %d \n",filenames[old_file_index],start_index,end_index,current_proc);
			

			if(actual_chunks_size>chunks_size){
				chunks_size *= 2;
				output_chunks = realloc(output_chunks,chunks_size*sizeof(chunk));	
				//printf("DBG: chunk resize\n");				
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


char** init_string_array(char **array,int size, int string_size){
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

int count_total_lines(int const *lines_array, int array_size){
	int sum = 0;
	for(int i = 0; i<array_size;i++){
		sum += lines_array[i];
	}	
	return sum;
}

int main(int argc, char *argv[]) {
	
	int my_rank,nofproc, local_file_count,total_lines = 0;
	int *file_sendcounts, *file_dspls,*local_file_lines_count = NULL, *global_file_lines_count;
	size_t n_of_files;
	char **filenames = NULL, **local_filenames = NULL, index_filename[FILENAME_SIZE];
	int send_index;			
	double start_time, end_time, time_elapsed;

	MPI_Init(&argc,&argv);
	MPI_Comm_rank(MPI_COMM_WORLD,&my_rank);
	MPI_Comm_size(MPI_COMM_WORLD,&nofproc);
	init_custom_types();	
	
	if(argc != 2){
		printf("Incorrect number of parameters!\n");
		exit(EXIT_FAILURE);
	}

	strcpy(index_filename,argv[1]);

	if(my_rank == MASTER){	
		start_time = MPI_Wtime();
		filenames = detect_files(filenames,&n_of_files,index_filename);		
		file_sendcounts = calloc(nofproc,sizeof(int));
		file_dspls = calloc(nofproc,sizeof(int));

		if(file_sendcounts == NULL || file_dspls == NULL){
			printf("Memory error !");
			exit(EXIT_FAILURE);
		}		

		divide_files(n_of_files,nofproc,file_sendcounts,file_dspls);		
		global_file_lines_count = calloc(n_of_files,sizeof(int));
		if(global_file_lines_count == NULL){
			printf("Memory error !");
			exit(EXIT_FAILURE);
		}
	}
		
	MPI_Scatter(file_sendcounts,1,MPI_INT,&local_file_count,1,MPI_INT,MASTER,MPI_COMM_WORLD);
	

	local_filenames = init_string_array(local_filenames,local_file_count,FILENAME_SIZE);
	
	if(my_rank == MASTER){
		send_index = file_sendcounts[0];
		for(int reciever = 1; reciever <nofproc;reciever++){
			for(int i = 0;i<file_sendcounts[reciever];i++){				
				MPI_Send(filenames[send_index],strlen(filenames[send_index])+1,MPI_CHAR,reciever,MASTER,MPI_COMM_WORLD);
				send_index++;
			}
		}

		for(int i = 0;i<local_file_count;i++){
			local_filenames[i] = strcpy(local_filenames[i],filenames[i]);			
		}

	} else {
		for(int i= 0; i<local_file_count;i++){
			MPI_Recv(local_filenames[i],FILENAME_SIZE,MPI_CHAR,MASTER,MPI_ANY_TAG,MPI_COMM_WORLD,MPI_STATUS_IGNORE);			
		}
	} 

	local_file_lines_count = calloc(local_file_count,sizeof(int));
	
	count_file_lines(local_filenames,local_file_lines_count,local_file_count);
	MPI_Gatherv(local_file_lines_count,local_file_count,MPI_INT,global_file_lines_count,file_sendcounts,file_dspls,MPI_INT,MASTER,MPI_COMM_WORLD);

	if(my_rank == MASTER){
		total_lines = count_total_lines(global_file_lines_count,nofproc);					
	}

	if(my_rank == MASTER){
		end_time = MPI_Wtime();
		time_elapsed = end_time - start_time;
		printf("Task took %lf seconds with %d processes on %d total lines\n",time_elapsed,nofproc,total_lines);
	}

	MPI_Finalize();
}