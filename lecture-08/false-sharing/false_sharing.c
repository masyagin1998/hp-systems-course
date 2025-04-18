// gcc -std=c11 -O2 -Wall -Wextra false_sharing.c -o false_sharing -lpthread
// ./false_sharing

// объяснить почему замедление в 15-20 раз, а не в 8.

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdatomic.h>
#include <time.h>

#define THREADS 8
#define ITERS 100000000ull
#define CACHE_LINE_SIZE 64

typedef struct {
    atomic_long value;
} shared_data_false;

shared_data_false data_false[THREADS];

typedef struct {
    atomic_long value;
    char padding[CACHE_LINE_SIZE - sizeof(atomic_long)];
} shared_data_true;

shared_data_true data_true[THREADS];

void* worker_false(void* arg) {
    int idx = *(int*)arg;
    for (size_t i = 0; i < ITERS; ++i)
        atomic_fetch_add_explicit(&data_false[idx].value, 1, memory_order_relaxed);
    return NULL;
}

void* worker_true(void* arg) {
    int idx = *(int*)arg;
    for (size_t i = 0; i < ITERS; ++i)
        atomic_fetch_add_explicit(&data_true[idx].value, 1, memory_order_relaxed);
    return NULL;
}

long timed_run(void *(*func)(void*)) {
    pthread_t threads[THREADS];
    int ids[THREADS];

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC_RAW, &start);

    for (int i = 0; i < THREADS; ++i) {
        ids[i] = i;
        pthread_create(&threads[i], NULL, func, &ids[i]);
    }
    for (int i = 0; i < THREADS; ++i)
        pthread_join(threads[i], NULL);

    clock_gettime(CLOCK_MONOTONIC_RAW, &end);

    long dur_ns = (end.tv_sec - start.tv_sec) * 1e9 + (end.tv_nsec - start.tv_nsec);
    return dur_ns;
}

int main() {
    long t1, t2;

    printf("Running no false sharing version...\n");
    t2 = timed_run(worker_true);
    printf("No false sharing time: %ld ns\n\n", t2);

    printf("Running false sharing version...\n");
    t1 = timed_run(worker_false);
    printf("False sharing time: %ld ns\n\n", t1);

    printf("Running no false sharing version...\n");
    t2 = timed_run(worker_true);
    printf("No false sharing time: %ld ns\n\n", t2);

    printf("Speedup: %.2fx\n", (double)t1 / t2);
    return 0;
}
