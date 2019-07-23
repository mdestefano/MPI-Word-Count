#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "wcutils.h"
#include "chunk.h"
#include "wordsmap.h"

#define MASTER 0

void init_custom_types(){
	initialize_chunk_type();
	initialize_woccurence_type();
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

void count_words_in_line(char *line,wordsmap *map){
	char* token; 
    char* rest = line; 
  
    while ((token = strtok_r(rest, " ,.-\n:!?()';\"\t", &rest))) {
			if(checkString(token)){
				token = string_to_lowercase(token);
				add_word(map,token);
			}
	}
}

wordsmap count_words_in_chunks(chunk* chunks, int nofchunks){	
	wordsmap output_map = new_wordsmap();
	char *line;
	size_t len = 512;
	line = calloc(len,sizeof(char));
	ssize_t line_size;
	FILE *curr_file;
	int line_index = 0;


	for (int i = 0; i < nofchunks; i++)	{				
		curr_file = fopen(chunks[i].filename,"r");
		line_index = 0;
		if(curr_file == NULL){
			printf("ERROR OPENING FILE!\n");
			exit(EXIT_FAILURE);
		}

		while(line_index<chunks[i].start_index){			
			getline(&line,&len,curr_file);
			line_index += 1;
		}

		while ((line_size = getline(&line,&len,curr_file)) != -1 && line_index <= chunks[i].end_index){			
			if(line[line_size-1] == '\n'){
				line[line_size-1] = '\0';
			}
			count_words_in_line(line,&output_map);
			line_index += 1;
		}
		
	}

	return output_map;
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
	wordsmap global_map, local_map;
	woccurrence *local_wocc_collection, *global_wocc_collection;
	int local_woccurrences_size, global_woccurrences_size, *global_woccurrences_size_count = NULL, *global_woccurrenecs_dspls = NULL;

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

		total_lines = wc_sum_array(global_file_lines_count,n_of_files);	
		
		#ifdef DEBUG
			printf("DBG: MASTER: Total lines = %d\n",total_lines);
		#endif // DEBUG

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

	
	local_map = count_words_in_chunks(local_chunks,local_nofchunks);

	local_wocc_collection = get_woccurrences_collection(local_map,&local_woccurrences_size);
	

	if(my_rank == MASTER){
		global_woccurrences_size_count = wc_init_int_array(global_woccurrences_size_count,nofproc);
		global_woccurrenecs_dspls = wc_init_int_array(global_woccurrenecs_dspls,nofproc);
	}

	MPI_Gather(&local_woccurrences_size,1,MPI_INT,global_woccurrences_size_count,1,MPI_INT,MASTER,MPI_COMM_WORLD);
	
	if(my_rank == MASTER){

		global_woccurrences_size = wc_sum_array(global_woccurrences_size_count,nofproc);			

		for(int i = 0, sum = 0;i<nofproc;i++){
			global_woccurrenecs_dspls[i] = sum;
			sum += global_woccurrences_size_count[i]; 
		}
		
		/*#ifdef DEBUG
	
		for(int i = 0; i < nofproc; i++){
			printf("global_woccurrences_size_count[%d]=%d, global_woccurrenecs_dspls[%d]=%d\n",i,global_woccurrences_size_count[i],i,global_woccurrenecs_dspls[i]);
			
		}
		printf("DBG: MASTER: Global map size=%d\n",global_woccurrences_size);
		#endif // DEBUG	*/

		global_wocc_collection = calloc(global_woccurrences_size,sizeof(woccurrence));
	}

	MPI_Gatherv(local_wocc_collection,local_woccurrences_size,mpi_woccurence_type,global_wocc_collection,global_woccurrences_size_count,global_woccurrenecs_dspls,mpi_woccurence_type,MASTER,MPI_COMM_WORLD);

	if(my_rank == MASTER){
		global_map = merge_woccurrences(global_wocc_collection,global_woccurrences_size);
		print_map(global_map);

		/*puts("TEST MAP MERGING");
		woccurrence test_occ[2];
		test_occ[0] = new_woccurence("prova");
		add_n_occurrence(&test_occ[0],2);
		test_occ[1] = new_woccurence("prova");

		wordsmap test_map = merge_woccurrences(test_occ,2);
		print_map(test_map);*/

		end_time = MPI_Wtime();
		time_elapsed = end_time - start_time;
		printf("Task took %lf seconds with %d processes on %d total lines\n",time_elapsed,nofproc,total_lines);
	}

	MPI_Finalize();
}