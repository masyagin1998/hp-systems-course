// gcc -std=c11 -O2 -Wall -Wextra -pthread mutex.c -o mutex
// ./mutex

#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

pthread_mutex_t mutex;
int shared_counter = 0;

void* thread_func(void* arg) {
    int id = *(int*)arg;

    for (int i = 0; i < 100; ++i) {
        pthread_mutex_lock(&mutex);
        printf("Thread %d\n", id);
        shared_counter++;
        usleep(10000);
        pthread_mutex_unlock(&mutex);
    }
    return NULL;
}

int main() {
    pthread_t t1, t2, t3, t4;
    int id0 = 0, id1 = 1, id2 = 2, id3 = 3;

    pthread_mutex_init(&mutex, NULL);

    pthread_create(&t1, NULL, thread_func, &id0);
    pthread_create(&t2, NULL, thread_func, &id1);
    pthread_create(&t3, NULL, thread_func, &id2);
    pthread_create(&t4, NULL, thread_func, &id3);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    pthread_join(t3, NULL);
    pthread_join(t4, NULL);

    pthread_mutex_destroy(&mutex);

    printf("Final counter value: %d\n", shared_counter);
    return 0;
}
