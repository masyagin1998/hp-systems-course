// gcc -std=c11 -O2 -Wall -Wextra -o semaphore semaphore.c -lrt
// ./semaphore
// strace ./semaphore
// strace -c -e write ./semaphore

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <semaphore.h>
#include <signal.h>

#define SHM_NAME "/shm_example_ipc"
#define SEM_CLIENT_READY "/sem_client_ready"
#define SEM_SERVER_READY "/sem_server_ready"
#define BUF_SIZE 1024
#define ITERATIONS 5

typedef struct {
    char buffer[BUF_SIZE];
} shared_mem_t;

void run_server(shared_mem_t *shm, sem_t *sem_client_ready, sem_t *sem_server_ready) {
    char temp[BUF_SIZE];

    for (int i = 0; i < ITERATIONS; ++i) {
        sem_wait(sem_client_ready);

        strncpy(temp, shm->buffer, BUF_SIZE);
        temp[BUF_SIZE - 1] = '\0';
        printf("%d got message \"%s\" from client\n", getpid(), temp);

        snprintf(shm->buffer, BUF_SIZE, "echo: %s", temp);

        sem_post(sem_server_ready);
    }
}

void run_client(shared_mem_t *shm, sem_t *sem_client_ready, sem_t *sem_server_ready) {
    char msg[BUF_SIZE];

    for (int i = 0; i < ITERATIONS; ++i) {
        snprintf(msg, sizeof(msg), "ping server %d", i);
        printf("%d sending message to server...\n", getpid());

        strncpy(shm->buffer, msg, BUF_SIZE);
        sem_post(sem_client_ready);

        sem_wait(sem_server_ready);

        printf("%d got message \"%s\" from server\n", getpid(), shm->buffer);
        sleep(1);
    }
}

int main() {
    sleep(5);

    int fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    ftruncate(fd, sizeof(shared_mem_t));
    shared_mem_t *shm = mmap(NULL, sizeof(shared_mem_t),
                             PROT_READ | PROT_WRITE,
                             MAP_SHARED, fd, 0);

    sleep(5);

    sem_t *sem_client_ready = sem_open(SEM_CLIENT_READY, O_CREAT, 0666, 0);
    sem_t *sem_server_ready = sem_open(SEM_SERVER_READY, O_CREAT, 0666, 0);
    pid_t pid = fork();

    if (pid == 0) {
        run_server(shm, sem_client_ready, sem_server_ready);
        _exit(0);
    } else {
        run_client(shm, sem_client_ready, sem_server_ready);
        wait(NULL);

        munmap(shm, sizeof(shared_mem_t));
        shm_unlink(SHM_NAME);
        sem_close(sem_client_ready);
        sem_close(sem_server_ready);
        sem_unlink(SEM_CLIENT_READY);
        sem_unlink(SEM_SERVER_READY);
    }

    return 0;
}
