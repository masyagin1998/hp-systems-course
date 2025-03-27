// gcc -std=c11 -O2 -Wall -Wextra -pthread livelock.c -o livelock
// ./livelock

#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdatomic.h>

atomic_int flag1 = 0;
atomic_int flag2 = 0;

void* thread1(void* arg) {
    (void)arg;
    while (1) {
        atomic_store(&flag1, 1);
        if (atomic_load(&flag2)) {
            printf("Thread 1 sees Thread 2 is active. Yielding...\n");
            atomic_store(&flag1, 0);
            usleep(100000);
            continue;
        }
        printf("Thread 1 is doing work.\n");
        usleep(100000);
        atomic_store(&flag1, 0);
    }
    return NULL;
}

void* thread2(void* arg) {
    (void)arg;
    while (1) {
        atomic_store(&flag2, 1);
        if (atomic_load(&flag1)) {
            printf("Thread 2 sees Thread 1 is active. Yielding...\n");
            atomic_store(&flag2, 0);
            usleep(100000);
            continue;
        }
        printf("Thread 2 is doing work.\n");
        usleep(100000);
        atomic_store(&flag2, 0);
    }
    return NULL;
}

int main(void) {
    pthread_t t1, t2;
    pthread_create(&t1, NULL, thread1, NULL);
    pthread_create(&t2, NULL, thread2, NULL);
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    return 0;
}
