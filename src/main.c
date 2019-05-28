#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "chunk.h"

void init_custom_types(){
	initialize_chunk_type();
}


int main(int argc, char *argv[]) {
	
	int my_rank,n_of_processors;

	MPI_Init(&argc,&argv);
	MPI_Comm_rank(MPI_COMM_WORLD,&my_rank);
	MPI_Comm_size(MPI_COMM_WORLD,&n_of_processors);
	init_custom_types();
	/*chunk a = new_chunk("",0,0);

	if(n_of_processors == 2){
		if(my_rank == 0){
			a = new_chunk("pippo",0,9);
			printf("Cunk %s\n",get_chunk_filename(a));
			MPI_Send(a,1,mpi_text_file_chunk,1,1,MPI_COMM_WORLD);
			printf("Chunk sent\n");
		} else {
			MPI_Recv(a,1,mpi_text_file_chunk,0,1,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
			printf("recieved chunk %s\n",get_chunk_filename(a));
		}
	}*/

	MPI_Finalize();
}