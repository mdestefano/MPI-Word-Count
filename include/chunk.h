
typedef struct chunk_c* chunk;

struct chunk_c {
  char* filename;
  size_t start_index;
  size_t end_index;
};



chunk new_chunk(const char filename[], size_t start_index,size_t end_index);
