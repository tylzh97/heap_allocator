#include <stdio.h>
#include <string.h> // For memset and memcpy
#include <stdlib.h> // For abort and other standard library functions
#include "include/heap.h"
#include <errno.h> // For ENOMEM

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
  if (size == 0)
  {
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
  if (realsize == 0)
  {
    return NULL;
  }
  fprintf(stderr, "Try to calloc(%ld, %ld) = %ld.\n", count, size, realsize);
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
  if (p == NULL)
  {
    fprintf(stderr, "free(NULL) called.\n");
    return;
  }

  fprintf(stderr, "free(%p) - allocated with malloc/calloc/realloc, freeing directly.\n", p);
  heap_free(&g_heap, p);
}

void *realloc(void *p, size_t size)
{
  init_allocator();

  fprintf(stderr, "Try to realloc(%p, %ld).\n", p, size);

  if (p == NULL)
  {
    void *ret = malloc(size);
    fprintf(stderr, "realloc(NULL, %ld) equivalent to malloc, returning %p.\n", size, ret);
    return ret; // realloc on NULL is same as malloc
  }

  if (size == 0)
  {
    free(p); // realloc to 0 is same as free and return NULL
    fprintf(stderr, "realloc(%p, 0) equivalent to free and returning NULL.\n", p);
    return NULL;
  }
  node_t *node = wrapper_get_node(p);
  size_t old_size = node->size;

  if (size <= old_size)
  {
    fprintf(stderr, "realloc(%p, %ld) requested size is <= old size, returning original pointer.\n", p, size);
    return p;
  }

  char *ret = heap_alloc(&g_heap, size);
  if (ret != NULL)
  {
    memcpy(ret, p, MIN(old_size, size)); // Copy the smaller of the two sizes
    free(p);
    fprintf(stderr, "realloc(%p, %ld) allocated new block %p, copied %ld bytes, and freed old block.\n", p, size, ret, MIN(old_size, size));
  }
  else
  {
    fprintf(stderr, "realloc(%p, %ld) allocation failed, returning NULL.\n", p, size);
    return NULL;
  }
  fprintf(stderr, "==> realloc(%p, %ld) = %p.\n", p, size, ret);
  return ret;
}

// Historically equivalent to free, but now deprecated.  Just call free.
void cfree(void *p)
{
  fprintf(stderr, "cfree(%p) called (equivalent to free).\n", p);
  free(p);
}

void *aligned_alloc(size_t alignment, size_t size)
{
  fprintf(stderr, "aligned_alloc(%ld, %ld) called, using malloc.\n", alignment, size);
  return malloc(size);
}

int posix_memalign(void **memptr, size_t alignment, size_t size)
{
  fprintf(stderr, "posix_memalign(0x%lx, %ld, %ld) called, using malloc.\n", (unsigned long)memptr, alignment, size);
  *memptr = malloc(size);
  return (*memptr == NULL) ? ENOMEM : 0;
}

void *memalign(size_t alignment, size_t size)
{
  fprintf(stderr, "memalign(%ld, %ld) called, using malloc.\n", alignment, size);
  return malloc(size);
}

void *valloc(size_t size)
{
  fprintf(stderr, "valloc(%ld) called, using malloc.\n", size);
  return malloc(size);
}

void *pvalloc(size_t size)
{
  fprintf(stderr, "pvalloc(%ld) called, using malloc.\n", size);
  return malloc(size);
}
