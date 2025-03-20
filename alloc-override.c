#include <stdio.h>
#include <string.h> // For memset and memcpy
#include "include/heap.h"

#define MIN(a, b) ((a) < (b) ? (a) : (b))

heap_t g_heap;
bin_t g_bins[BIN_COUNT] = {'\0'};
char g_region[HEAP_INIT_SIZE] = {'\0'};
int g_init_flag = 0;

// Initialization function
int init_allocator()
{
  if (!g_init_flag)
  {
    // fprintf(stderr, "Initializing heap.\n");
    for (int i = 0; i < BIN_COUNT; ++i)
    {
      g_heap.bins[i] = &(g_bins[i]);
    }
    init_heap(&g_heap, (long)(&g_region));
    g_init_flag = 1;
    // fprintf(stderr, "Heap initialized successfully.\n");
  }
  return 0; // Indicate success
}

void *malloc(size_t size)
{
  init_allocator();
  fprintf(stderr, "Try to malloc(%ld).\n", size);
  if (size == 0) {
    return NULL;
  }
  void *p = heap_alloc(&g_heap, size);
  fprintf(stderr, "==> malloc(%ld) = %p.\n", size, p);
  return p;
}

void *calloc(size_t count, size_t size)
{
  init_allocator();
  size_t realsize = count * size;
  if (realsize == 0) {
    return NULL;
  }
  fprintf(stderr, "Try to calloc(%ld).\n", realsize);
  char *p = heap_alloc(&g_heap, realsize);
  if (p != NULL)
  {                         // Check if allocation was successful before zeroing
    memset(p, 0, realsize); // Use memset for efficient zeroing
  }
  fprintf(stderr, "==> calloc(%ld) = %p.\n", realsize, p);
  return p;
}

node_t *wrapper_get_node(void *p)
{
  node_t *head = (node_t *)((char *)p - 8);
  return head;
}

void free(void *p)
{
  heap_free(&g_heap, p);
}

void *realloc(void *p, size_t size)
{
  init_allocator();

  fprintf(stderr, "Try to realloc(%p, %d).\n", p, size);

  if (p == NULL)
  {
    return malloc(size); // realloc on NULL is same as malloc
  }

  if (size == 0)
  {
    free(p); // realloc to 0 is same as free and return NULL
    return NULL;
  }
  node_t *node = wrapper_get_node(p);
  size_t old_size = node->size;

  if (size <= old_size) {
    return p;
  }

  char *ret = heap_alloc(&g_heap, size);
  if (ret != NULL) { 
    memcpy(ret, p, old_size); 
    free(p);
  } else {
    return NULL;
  }
  fprintf(stderr, "==> realloc(%p, %ld) = %p.\n", p, size, ret);
  return ret;
}

cfree(void *p) {}

aligned_alloc(size_t alignment, size_t size){}

posix_memalign(void **memptr, size_t alignment, size_t size) {}

memalign(size_t alignment, size_t size) {}

valloc(size_t size) {}

pvalloc(size_t size) {}
