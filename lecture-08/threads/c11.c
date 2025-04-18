// gcc -std=c11 -O2 -Wall -Wextra c11.c -o c11
// ./c11
// sudo apt install musl musl-tools
// musl-gcc -std=c11 -O2 -Wall -Wextra c11.c -o c11
// ./c11

#include <stdio.h>
#include <threads.h>

#define NUM_THREADS 4
#define INCREMENTS_PER_THREAD 100000

int counter = 0;
mtx_t mutex;

int increment(void *arg) {
    for (int i = 0; i < INCREMENTS_PER_THREAD; ++i) {
        mtx_lock(&mutex);
        counter++;
        mtx_unlock(&mutex);
    }
    return 0;
}

int main() {
    thrd_t threads[NUM_THREADS];
    mtx_init(&mutex, mtx_plain);

    for (int i = 0; i < NUM_THREADS; ++i) {
        thrd_create(&threads[i], increment, NULL);
    }

    for (int i = 0; i < NUM_THREADS; ++i) {
        thrd_join(threads[i], NULL);
    }

    mtx_destroy(&mutex);
    printf("Final counter: %d\n", counter);
    return 0;
}
