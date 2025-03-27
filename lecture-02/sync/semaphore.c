// gcc -std=c11 -O2 -Wall -Wextra -pthread semaphore.c -o semaphore
// ./semaphore

#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

sem_t sem;

void* writer(void* arg) {
    for (int i = 0; i < 5; ++i) {
        printf("Writer: producing data %d\n", i);
        sleep(1);
        sem_post(&sem);
    }
    return NULL;
}

void* reader(void* arg) {
    for (int i = 0; i < 5; ++i) {
        sem_wait(&sem);
        printf("Reader: consuming data %d\n", i);
    }
    return NULL;
}

int main() {
    pthread_t t1, t2;

    sem_init(&sem, 0, 0);

    pthread_create(&t1, NULL, writer, NULL);
    pthread_create(&t2, NULL, reader, NULL);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    sem_destroy(&sem);

    return 0;
}
