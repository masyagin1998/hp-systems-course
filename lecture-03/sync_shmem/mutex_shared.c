// gcc -std=c11 -O2 -Wall -Wextra -pthread -o mutex_shared mutex_shared.c
// ./mutex_shared

// попробуем убрать аттрибут shared, перекомпиллировать и перезапустить код

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/mman.h>
#include <sys/wait.h>

#define CHILDREN 4
#define ITERATIONS 100

typedef struct {
    pthread_mutex_t mutex;
    int counter;
} shared_data_t;

void child_process(shared_data_t* data, int id) {
    for (int i = 0; i < ITERATIONS; ++i) {
        pthread_mutex_lock(&data->mutex);
        printf("Process %d (pid %d)\n", id, getpid());
        data->counter++;
        usleep(10000);
        pthread_mutex_unlock(&data->mutex);
    }
    _exit(0);
}

int main() {
    shared_data_t* data = mmap(NULL, sizeof(shared_data_t),
                               PROT_READ | PROT_WRITE,
                               MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (data == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }

    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
    pthread_mutex_init(&data->mutex, &attr);
    pthread_mutexattr_destroy(&attr);

    data->counter = 0;

    pid_t pids[CHILDREN];
    for (int i = 0; i < CHILDREN; ++i) {
        if ((pids[i] = fork()) == 0) {
            child_process(data, i);
        }
    }

    for (int i = 0; i < CHILDREN; ++i) {
        wait(NULL);
    }

    printf("Final counter value: %d\n", data->counter);

    pthread_mutex_destroy(&data->mutex);
    munmap(data, sizeof(shared_data_t));

    return 0;
}
