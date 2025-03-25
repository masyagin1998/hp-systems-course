// gcc -g -Wall -Wextra -O2 huge.c -o huge
// ./huge

// cat /proc/meminfo | grep Huge
// sudo sysctl -w vm.nr_hugepages=64  # 64 * 2MB = 128MB
// cat /proc/meminfo | grep Huge
//
// cat /proc/self/smaps | grep -A20 Huge
// cat /proc/meminfo | grep Huge

// разобраться с gigantic

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>
#include <unistd.h>

#define ALLOC_SIZE (64 * (2 * 1024 * 1024UL))

int main() {
    void *ptr = mmap(NULL, ALLOC_SIZE,
                     PROT_READ | PROT_WRITE,
                     MAP_PRIVATE | MAP_ANONYMOUS | MAP_HUGETLB,
                     -1, 0);

    if (ptr == MAP_FAILED) {
        perror("mmap with MAP_HUGETLB failed");
        fprintf(stderr, "Maybe you need to set vm.nr_hugepages or run with sudo\n");
        return EXIT_FAILURE;
    }

    memset(ptr, 1, ALLOC_SIZE);
    printf("huge page memory allocated at: %p\n", ptr);
    pid_t pid = getpid();
    printf("PID: %d\n", pid);
    printf("grep -A20 '^%lx' /proc/%d/smaps\n", (uintptr_t)ptr, pid);

    getchar();

    munmap(ptr, ALLOC_SIZE);
    return 0;
}
