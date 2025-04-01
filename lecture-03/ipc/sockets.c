// gcc -std=c11 -O2 -Wall -Wextra -o sockets sockets.c
// ./sockets
// strace ./sockets
// strace -c -e write ./sockets

// переделать на epoll

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>

#define PORT 8888
#define BUF_SIZE 1024
#define ITERATIONS 5

void run_server() {
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    char buffer[BUF_SIZE];
    int opt = 1;

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    listen(server_fd, 1);

    client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);

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
}

void run_client() {
    sleep(1);

    int sockfd;
    struct sockaddr_in server_addr;
    char buffer[BUF_SIZE];
    char reply[BUF_SIZE];

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);
    connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr));

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
