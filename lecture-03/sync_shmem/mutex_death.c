// gcc -std=c11 -O2 -Wall -Wextra -pthread -o mutex_death mutex_death.c
// ./mutex_death

#define _GNU_SOURCE


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/wait.h>

typedef struct {
    pthread_mutex_t mutex;
    int value;
} shared_t;

int main() {
    shared_t* shm = mmap(NULL, sizeof(shared_t),
                         PROT_READ | PROT_WRITE,
                         MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (shm == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }

    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
    pthread_mutexattr_setrobust(&attr, PTHREAD_MUTEX_ROBUST);
    pthread_mutex_init(&shm->mutex, &attr);
    pthread_mutexattr_destroy(&attr);

    shm->value = 0;

    pid_t pid = fork();

    if (pid == 0) {
        if (pthread_mutex_lock(&shm->mutex) == 0) {
            printf("[child %d] locked mutex\n", getpid());
            shm->value = 42;
            printf("[child %d] set value = %d and exiting (simulate crash)\n", getpid(), shm->value);
            _exit(0);
        } else {
            perror("[child] mutex lock failed");
            _exit(1);
        }
    } else {
        waitpid(pid, NULL, 0);
        sleep(1);

        int rc = pthread_mutex_lock(&shm->mutex);
        if (rc == EOWNERDEAD) {
            printf("[parent %d] mutex owner is dead, data = %d\n", getpid(), shm->value);
            pthread_mutex_consistent(&shm->mutex);
            shm->value = 99;
            pthread_mutex_unlock(&shm->mutex);
        } else if (rc == 0) {
            printf("[parent %d] mutex locked normally, data = %d\n", getpid(), shm->value);
            pthread_mutex_unlock(&shm->mutex);
        } else {
            fprintf(stderr, "[parent] mutex_lock error: %s\n", strerror(rc));
        }

        printf("[parent %d] data = %d\n", getpid(), shm->value);

        pthread_mutex_destroy(&shm->mutex);
        munmap(shm, sizeof(shared_t));
    }

    return 0;
}
