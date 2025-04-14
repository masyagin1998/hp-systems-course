// gcc -std=c11 -O2 -Wall -Wextra -pthread processor_reordering.c -o processor_reordering
// ./processor_reordering

#include <stdio.h>
#include <pthread.h>
#include <stdatomic.h>

int x, y;
int r1, r2;

void* thread1(void* arg) {
    x = 1;
    r1 = y;
    return NULL;
}

void* thread2(void* arg) {
    y = 1;
    r2 = x;
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
