// gcc -g -Wall -Wextra -O2 malloc_free.c -o malloc_free
// strace ./malloc_free

// "The allocator maintains a cache of recently freed large chunks, to avoid frequent mmap/munmap cycles."
// — glibc malloc implementation notes

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define NUM_OBJECTS (100UL)
#define SMALL_ALLOC_SIZE (1024UL)
#define LARGE_ALLOC_SIZE (2 * 1024 * 1024UL)

int main() {
    void *ptrs[NUM_OBJECTS];

    size_t iter = 0;

    sleep(5);

    while (1) {
        for (size_t i = 0; i < NUM_OBJECTS; ++i) {
            if (i % 2 == 0) {
                ptrs[i] = malloc(SMALL_ALLOC_SIZE);
            } else {
                ptrs[i] = malloc(LARGE_ALLOC_SIZE);
            }

            if (!ptrs[i]) {
                perror("malloc");
                exit(EXIT_FAILURE);
            }
        }

        for (size_t i = 0; i < NUM_OBJECTS; ++i) {
            free(ptrs[i]);
        }

        printf("Iteration %zu\n", iter++);
        sleep(1);
    }

    return 0;
}
