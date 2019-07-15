#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "wcutils.h"
#include "chunk.h"
#include "wordsmap.h"

#define MASTER 0

void init_custom_types(){
	initialize_chunk_type();
	//initialize_woccurence_type();
}

char** detect_files(char **filenames, int *n,char *index_filename){
	int current_list_size = 16;
	size_t len = 0;
	ssize_t line_size;
	int i = 0;

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

			for(int j = i;j<current_list_size;j++){
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
		fclose(current_file);
	}
}

chunk* prepare_chunks(char **filenames, int *file_lines_count, int nofproc, int total_lines, int *output_nofchunks, int *chunk_sendcount, int* chunk_dspls){
	int lines_per_proc = total_lines/nofproc;
	int rem = total_lines % nofproc;
	int nofchunks = 4 * nofproc, sum = 0;
	int lines_left, file_index = 0,line_index = 0, chunk_index = 0, file_remaining_lines;
	chunk * output_chunks = NULL;
	int start_index, end_index;	
	char* current_filename;
		
	output_chunks = calloc (nofchunks,sizeof(chunk));	
	for(int i = 0;i < nofproc; i++){
		lines_left = (rem > 0) ? lines_per_proc + 1 : lines_per_proc;
		rem -= 1;			

		while(lines_left>0){			
			start_index = line_index;
			current_filename = filenames[file_index];
			file_remaining_lines = file_lines_count[file_index] - line_index;			
			
			if(lines_left >= file_remaining_lines){
				end_index = file_lines_count[file_index] - 1;
				line_index = 0;
				file_index += 1;
				lines_left -= (end_index - start_index + 1);
			} else {				
				line_index += lines_left;
				end_index = line_index - 1;
				lines_left = 0;
			}

			if(chunk_index == nofchunks){							
				nofchunks = nofchunks * 2;
				output_chunks = realloc(output_chunks,nofchunks);					
			}

			output_chunks[chunk_index] = new_chunk(current_filename,start_index,end_index);			
			chunk_index += 1;
			
			chunk_sendcount[i] += 1;
		}
		chunk_dspls[i] = sum;
		sum += chunk_sendcount[i];		
	}

	(*output_nofchunks) = chunk_index;
	return output_chunks;

}



int main(int argc, char *argv[]) {
	
	int my_rank,nofproc, local_file_count,total_lines = 0;
	int *file_sendcounts = NULL , *file_dspls = NULL, *local_file_lines_count = NULL, *global_file_lines_count = NULL;
	int n_of_files;
	char **filenames = NULL, **local_filenames = NULL, index_filename[FILENAME_SIZE];
	int send_index;			
	double start_time, end_time, time_elapsed;
	chunk* chunks = NULL, *local_chunks = NULL;
	int *chunk_sendcount = NULL, *chunk_dspls = NULL, nofchunks, local_nofchunks;

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

		file_sendcounts = wc_init_int_array(file_sendcounts,nofproc);
		file_dspls = wc_init_int_array(file_dspls,nofproc);
		global_file_lines_count = wc_init_int_array(global_file_lines_count,n_of_files);
			
		divide_files(n_of_files,nofproc,file_sendcounts,file_dspls);		
		
	}
		
	MPI_Scatter(file_sendcounts,1,MPI_INT,&local_file_count,1,MPI_INT,MASTER,MPI_COMM_WORLD);
	

	local_filenames = wc_init_string_array(local_filenames,local_file_count,FILENAME_SIZE);
	
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

	local_file_lines_count = wc_init_int_array(local_file_lines_count,local_file_count);	
	count_file_lines(local_filenames,local_file_lines_count,local_file_count);

	MPI_Gatherv(local_file_lines_count,local_file_count,MPI_INT,global_file_lines_count,file_sendcounts,file_dspls,MPI_INT,MASTER,MPI_COMM_WORLD);

	if(my_rank == MASTER){

		total_lines = wc_compute_total_lines(global_file_lines_count,n_of_files);	
		if(nofproc > total_lines){
			printf("ERROR: Number of processes excedes total lines to count. Use less processes!\n");
			MPI_Abort(MPI_COMM_WORLD,EXIT_FAILURE);
		}
		chunk_sendcount = wc_init_int_array(chunk_sendcount,nofproc);
		chunk_dspls = wc_init_int_array(chunk_dspls,nofproc);
						
		chunks = prepare_chunks(filenames,global_file_lines_count,nofproc,total_lines,&nofchunks,chunk_sendcount,chunk_dspls);	
		
		#ifdef DEBUG

		for(int i = 0; i < nofchunks; i++){
			print_chunk(chunks[i]);
			
		}

		for(int i = 0; i < nofproc; i++){
			printf("chunk_sendcount[%d]=%d, chunk_dspls[%d]=%d\n",i,chunk_sendcount[i],i,chunk_dspls[i]);
			
		}
		#endif // DEBUG	

	}

	MPI_Scatter(chunk_sendcount,1,MPI_INT,&local_nofchunks,1,MPI_INT,MASTER,MPI_COMM_WORLD);
	
	local_chunks = calloc(local_nofchunks,sizeof(chunk));

	MPI_Scatterv(chunks,chunk_sendcount,chunk_dspls,mpi_text_file_chunk,local_chunks,local_nofchunks,mpi_text_file_chunk,MASTER,MPI_COMM_WORLD);


	#ifdef DEBUG
	
	printf("DBG: process %d should recieve %d chunks\n",my_rank,local_nofchunks);
	for(int i = 0;i<local_nofchunks;i++){
		printf("DBG: process %d recieved ",my_rank);
		print_chunk(local_chunks[i]);
	}
	#endif // DEBUG

	

	if(my_rank == MASTER){
		puts("MAP test");
		wordsmap test = new_wordsmap();
		add_word(&test,"cacca");		
		add_word(&test,"pipi");		
		add_word(&test,"cacca");		
		add_word(&test,"termostato");		

		int wocc_dim;
		woccurrence *array = get_woccurrences_collection(test,&wocc_dim);
		for(int i = 0;i<wocc_dim;i++){
			print_occurrence(array[i]);
		}
						 

		end_time = MPI_Wtime();
		time_elapsed = end_time - start_time;
		printf("Task took %lf seconds with %d processes on %d total lines\n",time_elapsed,nofproc,total_lines);
	}

	MPI_Finalize();
}