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
	
	//---master
	//apri il file di indice e leggi i nomi dei files
	// calcola come distribuire i files
	
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