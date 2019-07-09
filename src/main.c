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
		printf("DBG: filename: %s\n",filenames[i]);
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
	printf("DBG: COUNTING LINES\n");
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
		printf("noflines %d\n",files_line_count[i]);
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

wordsmap count_words_in_chunks(chunk* chunks, int nofchunks,wordsmap output_map){
	FILE* curr_file;
	size_t curr_line_index,len = 512;
	ssize_t linesize;
	wordsmap map = new_wordsmap();
	char* curr_line = calloc(len,sizeof(char));
	char* single_fragment;	
	

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
			/*free(curr_line);
			curr_line = NULL;*/
		}

		while(curr_line_index <= get_chunk_end_index(chunks[curr_chunk]) && linesize != -1){											
			linesize = getline(&curr_line,&len,curr_file);			
			if(linesize != -1){
				//curr_line[linesize] = '\0';
				//printf("DBG: Current line: %s\n",curr_line);
				while ((single_fragment = strsep(&curr_line," .,-\n")) != NULL && strlen(single_fragment) >0){
					//printf("DBG: Fragment %s\n",single_fragment);
					add_word(map,single_fragment);
				}				
			}
			
			curr_line_index++;			
		} 		
		fclose(curr_file);
	}
	printf("DBG: words counted successfully\n");
	output_map = map;
	return output_map;
}


int main(int argc, char *argv[]) {
	
	int my_rank,nofproc, local_file_count,total_lines = 0;
	int *file_sendcounts, *file_dspls,*local_file_lines_count = NULL, *global_file_lines_count;
	size_t n_of_files,nofchunks;
	char **filenames = NULL, **local_filenames, index_filename[FILENAME_SIZE];
	int send_index;
	chunk *chunk_collection = NULL,*local_chunks = NULL;
	int *chunk_sendcount,*chunk_dspls;
	int local_chunk_count;
	wordsmap map = NULL, global_map = NULL;
	woccurrence *local_occurrences = NULL;
	woccurrence **global_occurrences = NULL;
	int *gobal_occurrences_sizes = NULL, local_occurrences_size;	
	double start_time, end_time;
	FILE* stats_file;


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
	//puts("DBG: COMUNICATION STARTING");
	MPI_Scatter(file_sendcounts,1,MPI_INT,&local_file_count,1,MPI_INT,MASTER,MPI_COMM_WORLD);
	//printf("DBG: proc. %d recv_count %d\n",my_rank,local_file_count);

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
		
	//puts("DBG: FILENAMES ARE ARRIVING!");
	if(my_rank == MASTER){
		for(int reciever = 1; reciever <nofproc;reciever++){
			for(int i = 0;i<file_sendcounts[reciever];i++){
				//printf("Master sending %s\n",filenames[send_index]);
				MPI_Send(filenames[send_index],strlen(filenames[send_index])+1,MPI_CHAR,reciever,MASTER,MPI_COMM_WORLD);
				send_index++;
			}
		}

		for(int i = 0;i<local_file_count;i++){
			local_filenames[i] = strcpy(local_filenames[i],filenames[i]);
			//printf("DBG: Master keeps %s\n",local_filenames[i]);
		}

	} else {
		for(int i= 0; i<local_file_count;i++){
			MPI_Recv(local_filenames[i],FILENAME_SIZE,MPI_CHAR,MASTER,MPI_ANY_TAG,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
			//printf("DBG: Process %d recieved %s\n",my_rank,local_filenames[i]);
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
		chunk_collection = prepare_chunks(filenames,global_file_lines_count,n_of_files,nofproc,chunk_collection,&nofchunks,chunk_sendcount,chunk_dspls,&total_lines);		
		puts("DBG: CHUNKS CREATED");
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
	puts("DBG: local chunks created");

	/* if chunk structure is changed not to be used as a pointer to structure,
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
	//print_map(map);
	local_occurrences_size = get_woccurrences_size(map);
	//printf("DBG: local_occurrences_size=%d\n",local_occurrences_size);
	local_occurrences = get_word_occurrences(map);
	
	
	if(my_rank == MASTER){
		//printf("DBG: MASTER: Starting reduce operation\n");
		global_occurrences = calloc(nofproc,sizeof(woccurrence*));	
		gobal_occurrences_sizes = calloc(nofproc,sizeof(int));					
		if(global_occurrences == NULL || gobal_occurrences_sizes == NULL){
			printf("MEMORY ERROR!\n");
			exit(EXIT_FAILURE);
		}		
	}
	MPI_Gather(&local_occurrences_size,1,MPI_INT,gobal_occurrences_sizes,1,MPI_INT,MASTER,MPI_COMM_WORLD);	
	if(my_rank == MASTER){		
		for(int i = 0;i<nofproc;i++){
			global_occurrences[i] = calloc(gobal_occurrences_sizes[i],sizeof(woccurrence));
			if(global_occurrences[i] == NULL){
				printf("MEMORY ERROR!\n");
				exit(EXIT_FAILURE);
			}
			for(int j = 0;j<gobal_occurrences_sizes[i];j++){
				global_occurrences[i][j] = new_woccurence("");
			}					
		}
		// TODO: improve performances with better data structures
		for(int i = 1;i<nofproc;i++){
			for(int j = 0;j<gobal_occurrences_sizes[i];j++){
				MPI_Recv(global_occurrences[i][j],1,mpi_woccurence_type,MPI_ANY_SOURCE,MPI_ANY_TAG,MPI_COMM_WORLD,MPI_STATUS_IGNORE);										
			}			
		}	
		for(int j = 0;j<gobal_occurrences_sizes[0];j++){
			global_occurrences[0][j] = local_occurrences[j];
			
		}	

		global_map = merge_wordoccuurences(global_occurrences,gobal_occurrences_sizes,nofproc);
		end_time = MPI_Wtime();
		print_map(global_map);
		printf("Job took %lf seconds with %d processes on %d lines\n",end_time-start_time,nofproc,total_lines);
		stats_file = fopen("output/stats.csv","a");
		if(stats_file == NULL){
			printf("FILE ERROR !\n");
		} else{
			fprintf(stats_file,"%d,%d,%lf\n",total_lines,nofproc,end_time-start_time);
			fclose(stats_file);
		}
	} else {		
		for(int i = 0;i<local_occurrences_size;i++){
			MPI_Send(local_occurrences[i],1,mpi_woccurence_type,MASTER,0,MPI_COMM_WORLD);
		}		
	}
	

	MPI_Finalize();
}