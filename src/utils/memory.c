#include "utils.h"
#include <stdio.h>

static size_t total_allocated = 0;
static size_t allocation_count = 0;

void *safe_malloc(size_t size) {
    if (size == 0) return NULL;

    void *ptr = malloc(size);
    if (ptr) {
        total_allocated += size;
        allocation_count++;
    }
    return ptr;
}

void safe_free(void *ptr) {
    if (ptr) {
        free(ptr);
        allocation_count--;
    }
}

int memory_check(void) {
    return (int)allocation_count;
}

void memory_stats(void) {
    printf("Memory Stats:\n");
    printf("  Total allocated: %zu bytes\n", total_allocated);
    printf("  Active allocations: %zu\n", allocation_count);
}

void memory_cleanup(void) {
    total_allocated = 0;
    allocation_count = 0;
}
