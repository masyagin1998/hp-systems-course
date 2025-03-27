// gcc -g -Wall -Wextra -O2 deadlock.c -o deadlock -pthread
// ./deadlock

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex2 = PTHREAD_MUTEX_INITIALIZER;

void* thread1(void* arg) {
    (void)arg;
    pthread_mutex_lock(&mutex1);
    printf("Thread 1 locked mutex1\n");
    sleep(1);
    pthread_mutex_lock(&mutex2);
    printf("Thread 1 locked mutex2\n");
    pthread_mutex_unlock(&mutex2);
    pthread_mutex_unlock(&mutex1);
    return NULL;
}

void* thread2(void* arg) {
    (void)arg;
    pthread_mutex_lock(&mutex2);
    printf("Thread 2 locked mutex2\n");
    sleep(1);
    pthread_mutex_lock(&mutex1);
    printf("Thread 2 locked mutex1\n");
    pthread_mutex_unlock(&mutex1);
    pthread_mutex_unlock(&mutex2);
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
