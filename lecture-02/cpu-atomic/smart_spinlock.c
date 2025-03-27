// gcc -std=c11 -O2 -Wall -Wextra -pthread smart_spinlock.c -o smart_spinlock
// ./smart_spinlock


#include <stdio.h>
#include <pthread.h>
#include <stdatomic.h>
#include <unistd.h>

typedef struct {
    atomic_int next_ticket;
    atomic_int now_serving;
} spinlock_t;

void spinlock_init(spinlock_t* lock) {
    atomic_init(&lock->next_ticket, 0);
    atomic_init(&lock->now_serving, 0);
}

void cpu_relax() {
    __asm__ __volatile__("pause");
}

void spinlock_lock(spinlock_t* lock) {
    int ticket = atomic_fetch_add(&lock->next_ticket, 1);
    while (atomic_load(&lock->now_serving) != ticket) {
        cpu_relax();  // добавлена pause-инструкция
    }
}

void spinlock_unlock(spinlock_t* lock) {
    atomic_fetch_add(&lock->now_serving, 1);
}

spinlock_t lock;
int shared_counter = 0;

void* thread_func(void* arg) {
    int id = *(int*)arg;

    for (int i = 0; i < 100; ++i) {
        spinlock_lock(&lock);
        printf("Thread %d\n", id);
        shared_counter++;
        usleep(10000);
        spinlock_unlock(&lock);
    }
    return NULL;
}

int main() {
    pthread_t t1, t2, t3, t4;
    int id0 = 0, id1 = 1, id2 = 2, id3 = 3;

    spinlock_init(&lock);

    pthread_create(&t1, NULL, thread_func, &id0);
    pthread_create(&t2, NULL, thread_func, &id1);
    pthread_create(&t3, NULL, thread_func, &id2);
    pthread_create(&t4, NULL, thread_func, &id3);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    pthread_join(t3, NULL);
    pthread_join(t4, NULL);

    printf("Final counter value: %d\n", shared_counter);
    return 0;
}
