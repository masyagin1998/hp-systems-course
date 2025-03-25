// gcc -g -Wall -Wextra -O2 malloc_mmap_brk.c -o malloc_mmap_brk
// strace ./malloc_mmap_brk

// ответить на вопросы:
// почему мы видим то brk, то mmap в strace?
// что было бы, если бы вместо malloc был jemalloc?

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define SMALL_ALLOC_SIZE (1024UL)
#define NUM_SMALL_ALLOCS (10000UL)

#define LARGE_ALLOC_SIZE (2 * 1024 * 1024UL)
#define NUM_LARGE_ALLOCS (100UL)

int main() {
    for (size_t i = 0; i < NUM_SMALL_ALLOCS; ++i) {
        void *ptr = malloc(SMALL_ALLOC_SIZE);
        if (!ptr) {
            perror("malloc (small)");
            exit(EXIT_FAILURE);
        }
    }

    for (size_t i = 0; i < NUM_LARGE_ALLOCS; ++i) {
        void *ptr = malloc(LARGE_ALLOC_SIZE);
        if (!ptr) {
            perror("malloc (large)");
            exit(EXIT_FAILURE);
        }
    }

    return 0;
}
