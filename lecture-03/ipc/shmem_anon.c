// gcc -std=c11 -O2 -Wall -Wextra -o shmem_anon shmem_anon.c
// ./shmem_anon
// strace ./shmem_anon
// strace -c -e write ./shmem_anon

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <signal.h>

#define BUF_SIZE 1024
#define ITERATIONS 5

typedef struct {
    volatile sig_atomic_t client_ready;
    volatile sig_atomic_t server_ready;
    char buffer[BUF_SIZE];
} shared_mem_t;

void run_server(shared_mem_t *shm) {
    char temp[BUF_SIZE];

    for (int i = 0; i < ITERATIONS; ++i) {
        while (!shm->client_ready) usleep(100);
        strncpy(temp, shm->buffer, BUF_SIZE);
        temp[BUF_SIZE - 1] = '\0';
        printf("%d got message \"%s\" from client\n", getpid(), temp);
        snprintf(shm->buffer, BUF_SIZE, "echo: %s", temp);

        shm->server_ready = 1;
        shm->client_ready = 0;
    }
}

void run_client(shared_mem_t *shm) {
    char msg[BUF_SIZE];

    for (int i = 0; i < ITERATIONS; ++i) {
        snprintf(msg, sizeof(msg), "ping server %d", i);
        printf("%d sending message to server...\n", getpid());

        while (shm->client_ready || shm->server_ready) usleep(100);
        strncpy(shm->buffer, msg, BUF_SIZE);
        shm->client_ready = 1;
        while (!shm->server_ready) usleep(100);

        printf("%d got message \"%s\" from server\n", getpid(), shm->buffer);
        shm->server_ready = 0;

        sleep(1);
    }
}

int main() {
    shared_mem_t *shm = mmap(NULL, sizeof(shared_mem_t),
                             PROT_READ | PROT_WRITE,
                             MAP_SHARED | MAP_ANONYMOUS,
                             -1, 0);
    if (shm == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }
    memset(shm, 0, sizeof(*shm));

    pid_t pid = fork();

    if (pid == 0) {
        run_server(shm);
        _exit(0);
    } else {
        run_client(shm);
        wait(NULL);
        munmap(shm, sizeof(shared_mem_t));
    }

    return 0;
}
