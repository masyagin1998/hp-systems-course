// gcc -std=c11 -O2 -Wall -Wextra -o mkfifo mkfifo.c
// ./mkfifo
// strace ./mkfifo
// strace -c -e write ./mkfifo

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/stat.h>

#define BUF_SIZE 1024
#define ITERATIONS 5

#define FIFO_C2S "/tmp/mkfifo_client_to_server"
#define FIFO_S2C "/tmp/mkfifo_server_to_client"

void run_server(int read_fd, int write_fd) {
    char buffer[BUF_SIZE];

    for (int i = 0; i < ITERATIONS; ++i) {
        ssize_t pos = 0;
        char c;
        while (read(read_fd, &c, 1) == 1 && c != '\0') {
            if (pos < BUF_SIZE - 1) {
                buffer[pos++] = c;
            }
        }
        buffer[pos] = '\0';

        printf("%d got message \"%s\" from client\n", getpid(), buffer);

        write(write_fd, buffer, strlen(buffer));
        write(write_fd, "\0", 1);
    }
}

void run_client(int write_fd, int read_fd) {
    char buffer[BUF_SIZE];
    char reply[BUF_SIZE];

    for (int i = 0; i < ITERATIONS; ++i) {
        snprintf(buffer, sizeof(buffer), "ping server %d", i);
        printf("%d sending message to server...\n", getpid());

        write(write_fd, buffer, strlen(buffer));
        write(write_fd, "\0", 1);

        ssize_t pos = 0;
        char c;
        while (read(read_fd, &c, 1) == 1 && c != '\0') {
            if (pos < BUF_SIZE - 1) {
                reply[pos++] = c;
            }
        }
        reply[pos] = '\0';

        printf("%d got message \"%s\" from server\n", getpid(), reply);
        sleep(1);
    }
}

int main() {
    // создаём FIFO, если ещё не существуют
    mkfifo(FIFO_C2S, 0666);
    mkfifo(FIFO_S2C, 0666);

    pid_t pid = fork();

    if (pid == 0) {
        // child = server
        int read_fd = open(FIFO_C2S, O_RDONLY);
        int write_fd = open(FIFO_S2C, O_WRONLY);

        run_server(read_fd, write_fd);

        close(read_fd);
        close(write_fd);
        _exit(0);
    } else {
        // parent = client
        int write_fd = open(FIFO_C2S, O_WRONLY);
        int read_fd = open(FIFO_S2C, O_RDONLY);

        run_client(write_fd, read_fd);

        close(write_fd);
        close(read_fd);
        wait(NULL);
    }

    // удаляем FIFO после завершения
    unlink(FIFO_C2S);
    unlink(FIFO_S2C);

    return 0;
}
