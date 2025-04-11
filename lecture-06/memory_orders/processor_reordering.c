// gcc -std=c11 -O2 -Wall -Wextra -pthread processor_reordering.c -o processor_reordering
// ./processor_reordering

#include <stdio.h>
#include <pthread.h>
#include <stdatomic.h>

atomic_int x, y;
int r1, r2;

void* thread1(void* arg) {
    // atomic_store(&x, 1);
    atomic_store_explicit(&x, 1, memory_order_seq_cst);
    // r1 = atomic_load(&y);
    r1 = atomic_load_explicit(&y, memory_order_seq_cst);
    return NULL;
}

void* thread2(void* arg) {
    // atomic_store(&y, 1);
    atomic_store_explicit(&y, 1, memory_order_seq_cst);
    // r2 = atomic_load(&x);
    r2 = atomic_load_explicit(&x, memory_order_seq_cst);
    return NULL;
}

int main() {
    int detected = 0;
    for (int i = 0; i < 1000000; ++i) {
        x = y = r1 = r2 = 0;

        pthread_t t1, t2;
        pthread_create(&t1, NULL, thread1, NULL);
        pthread_create(&t2, NULL, thread2, NULL);

        pthread_join(t1, NULL);
        pthread_join(t2, NULL);

        if (r1 == 0 && r2 == 0) {
            detected++;
            printf("Out-of-order detected! Iteration: %d\n", i);
            break;
        }
    }

    if (!detected) {
        printf("No out-of-order behavior detected.\n");
    }

    return 0;
}
