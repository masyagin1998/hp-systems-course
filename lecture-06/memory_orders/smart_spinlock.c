// gcc -std=c11 -O2 -Wall -Wextra -pthread smart_spinlock.c -o smart_spinlock
// ./smart_spinlock

// https://godbolt.org/
// использовать x86-64 gcc 14.2 -O3 и ARM 64 GCC 14.2.0 -O3...

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
    // int ticket = atomic_fetch_add(&lock->next_ticket, 1);
    int ticket = atomic_fetch_add_explicit(&lock->next_ticket, 1, memory_order_relaxed);
    // while (atomic_load(&lock->now_serving) != ticket) {
    while (atomic_load_explicit(&lock->now_serving, memory_order_acquire) != ticket) {
        cpu_relax();
    }
}

void spinlock_unlock(spinlock_t* lock) {
    // atomic_fetch_add(&lock->now_serving, 1);
    atomic_fetch_add_explicit(&lock->now_serving, 1, memory_order_release);
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
