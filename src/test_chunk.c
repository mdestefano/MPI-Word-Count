#include<stdio.h>
#include<assert.h>
#include<string.h>
#include<stdbool.h>
#include "../include/chunk.h"

void test_chunk(){

    chunk a_chunk = new_chunk("prova",0,10);
    assert(strcmp(a_chunk->filename,"prova") == true);
    assert(a_chunk->end_index == 0);
    assert(a_chunk->end_index == 10);



}