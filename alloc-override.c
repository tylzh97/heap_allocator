
#include <stdio.h>
#include "include/heap.h"

heap_t g_heap;
bin_t g_bins[BIN_COUNT] = {'\0'};
char g_region[HEAP_INIT_SIZE] = {'\0'};
int g_init_flag = 0;

void *malloc(size_t size)
{
  if (!g_init_flag)
  {
    // fprintf(stderr, "Try to init heap (1).\n");
    for (int i = 0; i < BIN_COUNT; ++i)
    {
      g_heap.bins[i] = &(g_bins[i]);
    }
    init_heap(&g_heap, (long)(&g_region));
    ++g_init_flag;
  }
  // fprintf(stderr, "Try to malloc(%ld).\n", size);
  return heap_alloc(&g_heap, size);
}

void *calloc(size_t count, size_t size)
{
  if (!g_init_flag)
  {
    // printf("Try to init heap (2).");
    for (int i = 0; i < BIN_COUNT; ++i)
    {
      g_heap.bins[i] = &(g_bins[i]);
    }
    init_heap(&g_heap, (long)(&g_region));
    ++g_init_flag;
  }
  size_t realsize = count * size;
  // fprintf(stderr, "Try to calloc(%ld).\n", realsize);
  char *p = heap_alloc(&g_heap, realsize);
  // fprintf(stderr, "Try to calloc(%ld) = %p.\n", realsize, p);
  for (int i = 0; i < realsize; ++i)
  {
    *p = '\0';
  }
  return p;
}

void *realloc(void *p, size_t size)
{
  char *ret = heap_alloc(&g_heap, size);
  for (int i = 0; i < size; ++i)
  {
    ret[i] = ((char *)p)[i];
  }
  return ret;
}

void free(void *p)
{
  heap_free(&g_heap, p);
}

//   void  cfree(void* p)                           MI_FORWARD0(mi_free,p)

//   void* valloc(size_t size)                      { return mi_valloc(size); }
//   void* pvalloc(size_t size)                     { return mi_pvalloc(size); }
//   void* memalign(size_t alignment, size_t size)  { return mi_memalign(alignment,size); }
//   int   __posix_memalign(void** p, size_t alignment, size_t size) { return mi_posix_memalign(p,alignment,size); }
