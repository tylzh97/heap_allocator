#include <stdio.h>
#include <string.h> // For memset and memcpy
#include <stdlib.h> // For abort and other standard library functions
#include "include/heap.h"
#include <errno.h> // For ENOMEM

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define ALIGNED_ALLOC_MAGIC 0x12345678

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

  size_t* metadata_ptr = (size_t*)(p - sizeof(size_t) * 2);
  if (metadata_ptr[0] == ALIGNED_ALLOC_MAGIC) {
    void *original_ptr = (void*)metadata_ptr[1];
    heap_free(&g_heap, original_ptr);
    return;
  }

  fprintf(stderr, "free(%p) - allocated with malloc/calloc/realloc, freeing directly.\n", p);
  heap_free(&g_heap, p);
  fprintf(stderr, "free(%p) Success!\n", p);
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


void *aligned_alloc_custom(size_t alignment, size_t size) {
  if (alignment == 0 || (alignment & (alignment - 1)) != 0) {
    errno = EINVAL;
    return NULL; // Alignment must be a power of 2
  }

  if (size == 0) return NULL; // malloc(0) 的行为依赖于实现

  // 1. Allocate extra memory to ensure alignment can be met and store metadata.
  size_t total_size = size + 2 * alignment + sizeof(size_t) * 2; // Extra space for alignment + metadata

  void *ptr = malloc(total_size);
  if (ptr == NULL) {
    return NULL;
  }

  // 2. Calculate the aligned address.
  uintptr_t raw_address = (uintptr_t)ptr;
  uintptr_t aligned_address = (raw_address + alignment + sizeof(size_t) * 2) & ~(alignment - 1);

  // 3. Store the original pointer and magic number before the aligned address.
  size_t* metadata_ptr = (size_t*)(aligned_address - sizeof(size_t) * 2);
  metadata_ptr[0] = ALIGNED_ALLOC_MAGIC;
  metadata_ptr[1] = (size_t)ptr; // Store the original pointer

  // 4. Return the aligned address.
  return (void *)aligned_address;
}

int posix_memalign(void **memptr, size_t alignment, size_t size)
{
  fprintf(stderr, "posix_memalign(0x%lx, %ld, %ld) called, using malloc.\n", (unsigned long)memptr, alignment, size);
  *memptr = aligned_alloc_custom(alignment, size);
  return (*memptr == NULL) ? ENOMEM : 0;
}

void *memalign(size_t alignment, size_t size)
{
  fprintf(stderr, "memalign(%ld, %ld) called, using malloc.\n", alignment, size);
  return aligned_alloc_custom(alignment, size);
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
