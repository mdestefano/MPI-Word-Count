#include <stdio.h>
#include <chunk.h>
#include <assert.h>
#include <string.h>

int main(int argc, char *argv[]) {
	chunk a_chunk = new_chunk("prova",0,10);
	chunk another = new_chunk("cacca",0,10);
	assert(!chunk_equals(a_chunk,another));
}