// gcc -g -Wall -Wextra -O2 cache_sizes_measure.c -o cache_sizes_measure
// pip install plotly
// ./cache_sizes_measure | python plot.py

// достаём кэши как:
// lscpu | grep -i "cache"
// cat /sys/devices/system/cpu/cpu*/cache/index*/size
// index0 -> L1d, index1 -> L1i, index2 -> L2, index3 -> L3

// ответить на вопросы:
// почему результаты по кэшам такие неправдоподобные? как это исправить?
// подсказки:
// - нам мешают другие процессы вокруг нас;
// - мы перескакиваем с ядра на ядро;
// - ядро на котором мы находимся может быть использовано для чего-то ещё.
// попробуйте применить perf и т.д.

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define N_SAMPLES (5UL)

#define MIN_SIZE (1024UL)
#define MAX_SIZE (128 * 1024 * 1024UL)

#define MIN_STRIDE (1UL)
#define MAX_STRIDE (1024UL)

double get_elapsed(clock_t start, clock_t end) {
    return (double)(end - start) / (double)(CLOCKS_PER_SEC);
}

int main() {
    for (size_t size = MIN_SIZE; size <= MAX_SIZE; size *= 2) {
        for (size_t stride = MIN_STRIDE; stride <= MAX_STRIDE; stride *= 2) {
            double elapsed = 0.0;

            for (size_t _ = 0; _ < N_SAMPLES; ++_) {
                int* arr = (int *) malloc(size * sizeof(int));
                clock_t start = clock();
                for (size_t iters = 0; iters < MAX_SIZE; iters += size / stride)
                    for (size_t i = 0; i < size; i += stride)
                        arr[i] += 1;
                double elapsed_with_mem_access = get_elapsed(start, clock());
                free(arr);

                register int dummy = 0;
                start = clock();
                for (size_t iters = 0; iters < MAX_SIZE; iters += size / stride)
                    for (size_t i = 0; i < size; i += stride)
                        dummy += 1;
                double elapsed_with_reg_access = get_elapsed(start, clock());

                elapsed += elapsed_with_mem_access - elapsed_with_reg_access;
            }

            double ns_per_access = elapsed * 1.0e9 / (double)N_SAMPLES / (double)(MAX_SIZE);
            printf("%lu,%lu,%f\n", size * sizeof(int), stride * sizeof(int), ns_per_access);
        }
    }
}