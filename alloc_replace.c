#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <dlfcn.h>

//contains address returned by *alloc functions / passed to free
// for realloc it's memory passed to it, not returned
void* g_alloc_mem = NULL;
// size passed to last malloc
size_t g_alloc_ammount = 0;


//number of malloc calls
extern int MALLOC_NUM;


void* malloc_replace(size_t size){
  if(MALLOC_NUM-- == 0) return NULL;
  void* mem =  malloc(size);
  g_alloc_ammount = size;
  g_alloc_mem = mem;
  return mem;
}

void free_replace(void* ptr){
  g_alloc_mem = ptr;
  free(ptr);
}

void* calloc_replace(size_t nmemb, size_t size){
  if(MALLOC_NUM-- == 0) return NULL;

  g_alloc_ammount = size * nmemb;
  void* mem = calloc(nmemb, size);
  g_alloc_mem = mem;
  return mem;
}

void* realloc_replace(void* ptr, size_t size){
  if(MALLOC_NUM-- == 0) return NULL;
  g_alloc_ammount = size;
  g_alloc_mem = ptr;
  return realloc(ptr, size);
}

//NOTE afaik reallocarray is gnu extension
void* reallocarray_replace(void* ptr, size_t nmemb, size_t size){
  if(MALLOC_NUM-- == 0) return NULL;
  g_alloc_ammount = size * nmemb;
  g_alloc_mem = ptr;
  return realloc(ptr, nmemb * size);
}



void* fopen_replace(const char *restrict pathname, const char *restrict mode){
  if(MALLOC_NUM-- == 0) return NULL;
  return fopen(pathname, mode);
}

#define malloc(size) malloc_replace(size)
#define free(mem) free_replace(mem)
#define calloc(nmemb, size) calloc_replace(nmemb, size)
#define realloc(ptr, size) realloc_replace(ptr, size)
#define reallocarray(ptr, nmemb, size) reallocarray_replace(ptr, nmemb, size)

#define fopen(path, mode) fopen_replace(path, mode)
