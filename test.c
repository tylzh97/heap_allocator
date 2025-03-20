#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>

// 定义测试的迭代次数和分配大小
#define NUM_ITERATIONS 100000 // 增加迭代次数以更准确地测量吞吐量
#define ALLOCATION_SIZE 1024 // 1KB

// 函数指针类型，用于指向 malloc, calloc, realloc
typedef void* (*alloc_func_t)(size_t size);
typedef void* (*realloc_func_t)(void *ptr, size_t size);
typedef void (*free_func_t)(void *ptr);

// 测试吞吐量的通用函数
double test_allocator_throughput(const char *allocator_name, alloc_func_t alloc_func, realloc_func_t realloc_func, free_func_t free_func) {
    clock_t start_time, end_time;
    double cpu_time_used;
    void *pointers[NUM_ITERATIONS]; // 存储分配的指针，用于后续 free
    size_t total_bytes_allocated = 0;

    start_time = clock();

    // 执行大量的分配和释放操作
    for (int i = 0; i < NUM_ITERATIONS; ++i) {
        void *ptr = alloc_func(ALLOCATION_SIZE);
        assert(ptr != NULL); // 确保分配成功
        pointers[i] = ptr;
        total_bytes_allocated += ALLOCATION_SIZE;
    }

    // 释放所有分配的内存
    for (int i = 0; i < NUM_ITERATIONS; ++i) {
        free_func(pointers[i]);
    }

    end_time = clock();
    cpu_time_used = ((double) (end_time - start_time)) / CLOCKS_PER_SEC;

    double throughput_ops_per_sec = (double)NUM_ITERATIONS / cpu_time_used;
    double throughput_bytes_per_sec = (double)total_bytes_allocated / cpu_time_used;

    printf("Allocator: %s\n", allocator_name);
    printf("Iterations: %d\n", NUM_ITERATIONS);
    printf("Allocation Size: %zu bytes\n", ALLOCATION_SIZE);
    printf("Total Bytes Allocated: %zu bytes\n", total_bytes_allocated);
    printf("Time taken: %.4f seconds\n", cpu_time_used);
    printf("Throughput (Operations/sec): %.2f ops/sec\n", throughput_ops_per_sec);
    printf("Throughput (Bytes/sec): %.2f bytes/sec\n", throughput_bytes_per_sec);
    printf("------------------------------------\n");

    return throughput_ops_per_sec; // 返回操作吞吐量，可以用于比较
}

// calloc 需要两个参数，这里创建一个 wrapper 函数来适配 alloc_func_t
void* calloc_wrapper(size_t size) {
    return calloc(1, size);
}

// 测试 calloc 的吞吐量
double test_calloc_throughput() {
    return test_allocator_throughput("calloc", (alloc_func_t)calloc_wrapper, NULL, free);
}

// 测试 malloc 的吞吐量
double test_malloc_throughput() {
    return test_allocator_throughput("malloc", malloc, NULL, free);
}

// 测试 realloc 的吞吐量 (这里简化为 realloc 同等大小的块)
double test_realloc_throughput() {
    clock_t start_time, end_time;
    double cpu_time_used;
    void *pointers[NUM_ITERATIONS];
    size_t total_bytes_allocated = 0;

    // 先用 malloc 分配初始内存
    for (int i = 0; i < NUM_ITERATIONS; ++i) {
        pointers[i] = malloc(ALLOCATION_SIZE);
        assert(pointers[i] != NULL);
    }

    start_time = clock();

    // 执行大量的 realloc 操作，每次 realloc 到相同的大小
    for (int i = 0; i < NUM_ITERATIONS; ++i) {
        void *ptr = realloc(pointers[i], ALLOCATION_SIZE);
        assert(ptr != NULL);
        pointers[i] = ptr; // 更新指针
        total_bytes_allocated += ALLOCATION_SIZE; // 每次 realloc 也算作一次分配
    }

    // 释放所有分配的内存
    for (int i = 0; i < NUM_ITERATIONS; ++i) {
        free(pointers[i]);
    }

    end_time = clock();
    cpu_time_used = ((double) (end_time - start_time)) / CLOCKS_PER_SEC;

    double throughput_ops_per_sec = (double)NUM_ITERATIONS / cpu_time_used;
    double throughput_bytes_per_sec = (double)total_bytes_allocated / cpu_time_used;

    printf("Allocator: realloc (same size)\n");
    printf("Iterations: %d\n", NUM_ITERATIONS);
    printf("Allocation Size: %zu bytes\n", ALLOCATION_SIZE);
    printf("Total Bytes Allocated (realloc): %zu bytes\n", total_bytes_allocated); // 注意这里是 realloc 的总字节数
    printf("Time taken: %.4f seconds\n", cpu_time_used);
    printf("Throughput (Operations/sec): %.2f ops/sec\n", throughput_ops_per_sec);
    printf("Throughput (Bytes/sec): %.2f bytes/sec\n", throughput_bytes_per_sec);
    printf("------------------------------------\n");

    return throughput_ops_per_sec;
}

// 测试 free 的吞吐量 (通常 free 的吞吐量是隐含在 malloc/calloc/realloc 测试中的，这里可以单独测试，但意义可能不大)
double test_free_throughput() {
    clock_t start_time, end_time;
    double cpu_time_used;
    void *pointers[NUM_ITERATIONS];

    // 先用 malloc 分配内存
    for (int i = 0; i < NUM_ITERATIONS; ++i) {
        pointers[i] = malloc(ALLOCATION_SIZE);
        assert(pointers[i] != NULL);
    }

    start_time = clock();

    // 执行大量的 free 操作
    for (int i = 0; i < NUM_ITERATIONS; ++i) {
        free(pointers[i]);
    }

    end_time = clock();
    cpu_time_used = ((double) (end_time - start_time)) / CLOCKS_PER_SEC;

    double throughput_ops_per_sec = (double)NUM_ITERATIONS / cpu_time_used;

    printf("Allocator: free\n");
    printf("Iterations: %d\n", NUM_ITERATIONS);
    printf("Allocation Size: %zu bytes (allocated before free test)\n", ALLOCATION_SIZE);
    printf("Time taken: %.4f seconds\n", cpu_time_used);
    printf("Throughput (Frees/sec): %.2f frees/sec\n", throughput_ops_per_sec);
    printf("------------------------------------\n");

    return throughput_ops_per_sec;
}


int main() {
    printf("Starting memory allocator throughput tests...\n\n");

    test_malloc_throughput();
    test_calloc_throughput();
    test_realloc_throughput();
    test_free_throughput(); // 可选，free 的吞吐量通常隐含在其他测试中

    printf("\nMemory allocator throughput tests completed.\n");

    return 0;
}