// gcc -std=c11 -O2 -Wall -Wextra -lc -D_XOPEN_SOURCE=500 -pthread peterson.c -o peterson
// ./peterson

// разница volatile и atomic
// попробуйте убрать atomic и поменять его на volatile, по идее мьютекс реализован по прежнему корректно,
// так как мы пишем в выровненную память, но нет...

#include <stdio.h>
#include <pthread.h>
#include <stdatomic.h>
#include <unistd.h>

typedef struct {
    atomic_int flag[2];
    atomic_int turn;
} peterson_mutex_t;

void peterson_lock(peterson_mutex_t* mutex, int self) {
    int other = 1 - self;
    atomic_store(&mutex->flag[self], 1);
    atomic_store(&mutex->turn, other);
    while (atomic_load(&mutex->flag[other]) && atomic_load(&mutex->turn) == other) {
        // spin
    }
}

void peterson_unlock(peterson_mutex_t* mutex, int self) {
    atomic_store(&mutex->flag[self], 0);
}

peterson_mutex_t mutex = { .flag = {0, 0}, .turn = 0 };
int shared_counter = 0;

void* thread_func(void* arg) {
    int id = *(int*)arg;

    for (int i = 0; i < 100; ++i) {
        peterson_lock(&mutex, id);
        printf("Thread %d\n", id);
        shared_counter++;
        usleep(10000);
        peterson_unlock(&mutex, id);
    }

    return NULL;
}

int main() {
    pthread_t t1, t2;
    int id0 = 0, id1 = 1;

    pthread_create(&t1, NULL, thread_func, &id0);
    pthread_create(&t2, NULL, thread_func, &id1);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    printf("Final counter value: %d\n", shared_counter);

    return 0;
}
