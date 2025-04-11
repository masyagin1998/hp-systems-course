// gcc -std=c11 -O2 -Wall -Wextra -pthread stupid_spinlock.c -o stupic_spinlock
// ./stupid_spinlock

// https://godbolt.org/
// использовать x86-64 gcc 14.2 -O3 и ARM 64 GCC 14.2.0 -O3...


#include <stdio.h>
#include <pthread.h>
#include <stdatomic.h>
#include <unistd.h>

typedef struct {
    atomic_flag flag;
} spinlock_t;

void spinlock_init(spinlock_t* lock) {
    // atomic_flag_clear(&lock->flag);
    atomic_flag_clear_explicit(&lock->flag, memory_order_relaxed);
}

void spinlock_lock(spinlock_t* lock) {
    // while (atomic_flag_test_and_set(&lock->flag)) {
    while (atomic_flag_test_and_set_explicit(&lock->flag, memory_order_acquire)) {
    }
}

void spinlock_unlock(spinlock_t* lock) {
    // atomic_flag_clear(&lock->flag);
    atomic_flag_clear_explicit(&lock->flag, memory_order_release);
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
