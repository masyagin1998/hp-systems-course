// gcc -std=c11 -O2 -Wall -Wextra -o queue queue.c -lrt
// ./queue
// strace ./queue
// strace -c -e write ./queue

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <mqueue.h>
#include <sys/wait.h>

#define MQ_NAME_C2S "/mq_c2s"
#define MQ_NAME_S2C "/mq_s2c"
#define BUF_SIZE 1024
#define ITERATIONS 5

void run_server() {
    struct mq_attr attr = {
        .mq_flags = 0,
        .mq_maxmsg = 10,
        .mq_msgsize = BUF_SIZE,
        .mq_curmsgs = 0
    };

    mqd_t c2s = mq_open(MQ_NAME_C2S, O_RDONLY | O_CREAT, 0666, &attr);
    mqd_t s2c = mq_open(MQ_NAME_S2C, O_WRONLY | O_CREAT, 0666, &attr);

    char buffer[BUF_SIZE];
    for (int i = 0; i < ITERATIONS; ++i) {
        ssize_t n = mq_receive(c2s, buffer, BUF_SIZE, NULL);
        if (n <= 0) {
            break;
        }

        buffer[n] = '\0';
        printf("%d got message \"%s\" from client\n", getpid(), buffer);

        mq_send(s2c, buffer, strlen(buffer), 0);
    }

    mq_close(c2s);
    mq_close(s2c);
    mq_unlink(MQ_NAME_C2S);
    mq_unlink(MQ_NAME_S2C);
}

void run_client() {
    sleep(1);  // дать серверу время открыть

    struct mq_attr attr;
    mqd_t c2s = mq_open(MQ_NAME_C2S, O_WRONLY);
    mqd_t s2c = mq_open(MQ_NAME_S2C, O_RDONLY);

    char buffer[BUF_SIZE];
    char reply[BUF_SIZE];

    for (int i = 0; i < ITERATIONS; ++i) {
        snprintf(buffer, sizeof(buffer), "ping server %d", i);
        printf("%d sending message to server...\n", getpid());

        mq_send(c2s, buffer, strlen(buffer), 0);

        ssize_t n = mq_receive(s2c, reply, BUF_SIZE, NULL);
        if (n <= 0) {
            break;
        }
        reply[n] = '\0';

        printf("%d got message \"%s\" from server\n", getpid(), reply);
        sleep(1);
    }

    mq_close(c2s);
    mq_close(s2c);
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
