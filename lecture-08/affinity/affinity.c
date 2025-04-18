// gcc -std=c11 -O2 -Wall -Wextra affinity.c -o affinity
// ./affinity

// cat /proc/interrupts

#define _GNU_SOURCE
#include <stdio.h>
#include <sched.h>
#include <unistd.h>
#include <time.h>

int main() {
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(0, &cpuset);
    CPU_SET(1, &cpuset);

    sched_setaffinity(0, sizeof(cpuset), &cpuset);

    for (int i = 0; i < 100; ++i) {
        printf("Running on CPU %d\n", sched_getcpu());
        struct timespec ts = {0, 100 * 1000000}; // 100 ms
        nanosleep(&ts, NULL);
    }

    volatile int sink = 0;
    while (1) {
        ++sink;
    }
}
