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
  // fprintf(stderr, "Try to malloc(%ld).\n", size);
  return heap_alloc(&g_heap, size);
}

void *calloc(size_t count, size_t size)
{
  init_allocator();
  size_t realsize = count * size;
  // fprintf(stderr, "Try to calloc(%ld).\n", realsize);
  char *p = heap_alloc(&g_heap, realsize);
  if (p != NULL)
  {                         // Check if allocation was successful before zeroing
    memset(p, 0, realsize); // Use memset for efficient zeroing
  }
  // fprintf(stderr, "Try to calloc(%ld) = %p.\n", realsize, p);
  return p;
}

node_t *wrapper_get_node(void *p)
{
  node_t *head = (node_t *)((char *)p - 8);
  return head;
}

void *realloc(void *p, size_t size)
{
  init_allocator();

  if (p == NULL)
  {
    return malloc(size); // realloc on NULL is same as malloc
  }

  if (size == 0)
  {
    free(p); // realloc to 0 is same as free and return NULL
    return NULL;
  }

  char *ret = heap_alloc(&g_heap, size);
  if (ret != NULL)
  { // Check if allocation was successful before copying
    node_t *node = wrapper_get_node(p);
    size_t old_size = node->size;

    memcpy(ret, p, MIN(size, old_size)); // Use memcpy for efficient copying
    free(p);                             // Free the old pointer after successful reallocation
  }
  return ret;
}

void free(void *p)
{
  heap_free(&g_heap, p);
}
