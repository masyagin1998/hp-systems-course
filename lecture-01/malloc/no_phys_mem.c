// gcc -g -Wall -Wextra -O2 no_phys_mem.c -o no_phys_mem
// strace ./no_phys_mem

// "The allocator maintains a cache of recently freed large chunks, to avoid frequent mmap/munmap cycles."
// — glibc malloc implementation notes

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define ARR_SIZE (8 * 1024 * 1024 * 1024UL)

int main(void) {
    printf("Allocating %zu bytes...\n", ARR_SIZE);
    char *p = malloc(ARR_SIZE);
    if (!p) {
        perror("malloc");
        return 1;
    }

    printf("Sleeping for 10 seconds (memory not touched yet)...\n");
    sleep(10);

    printf("Touching the last byte of the allocated memory...\n");
    p[ARR_SIZE - 1] = 42;

    printf("Sleeping for 10 more seconds...\n");
    sleep(10);

    printf("Filling entire array to trigger physical memory allocation...\n");
    size_t sum = 0;
    for (size_t i = 0; i < ARR_SIZE; i++) {
        p[i] = (char)(i % 256);
        sum += p[i];
    }
    printf("Done filling. Sum: %zu\n", sum);

    printf("Sleeping for 10 more seconds...\n");
    sleep(10);

    printf("Freeing memory...\n");
    free(p);

    return 0;
}
