// gcc -std=c11 -O2 -Wall -Wextra -o unix_sockets unix_sockets.c
// ./unix_sockets
// strace ./unix_sockets
// strace -c -e write ./unix_sockets

// переделать на epoll

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <sys/wait.h>

#define SOCKET_PATH "/tmp/unix_socket_example"
#define BUF_SIZE 1024
#define ITERATIONS 5

void run_server() {
    int server_fd, client_fd;
    struct sockaddr_un addr;
    char buffer[BUF_SIZE];

    unlink(SOCKET_PATH);
    server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);
    bind(server_fd, (struct sockaddr *)&addr, sizeof(addr));
    listen(server_fd, 1);

    client_fd = accept(server_fd, NULL, NULL);

    for (int i = 0; i < ITERATIONS; ++i) {
        ssize_t n = read(client_fd, buffer, BUF_SIZE - 1);
        if (n <= 0) {
            break;
        }
        buffer[n] = '\0';
        printf("%d got message \"%s\" from client\n", getpid(), buffer);
        write(client_fd, buffer, strlen(buffer));
    }

    close(client_fd);
    close(server_fd);

    unlink(SOCKET_PATH);
}

void run_client() {
    sleep(1);

    int sockfd;
    struct sockaddr_un addr;
    char buffer[BUF_SIZE];
    char reply[BUF_SIZE];

    sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);
    connect(sockfd, (struct sockaddr *)&addr, sizeof(addr));

    for (int i = 0; i < ITERATIONS; ++i) {
        printf("%d sending message to server...\n", getpid());
        snprintf(buffer, sizeof(buffer), "ping server %d", i);
        write(sockfd, buffer, strlen(buffer));

        ssize_t n = read(sockfd, reply, BUF_SIZE - 1);
        if (n <= 0) {
            break;
        }
        reply[n] = '\0';

        printf("%d got message \"%s\" from server\n", getpid(), reply);
        sleep(1);
    }

    close(sockfd);
}

int main() {
    pid_t pid = fork();

    if (pid == 0) {
        run_server();
        _exit(0);
    } else {
        run_client();
        wait(NULL);
    }

    return 0;
}
