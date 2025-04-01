// gcc -std=c11 -O2 -Wall -Wextra -pthread fork.c -o fork
// ./fork

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main() {
    printf("%d hello from parent\n", getpid());

    pid_t pid = fork();
    if (pid == 0) {
        printf("%d hello from child\n", getpid());
        sleep(6);
    } else {
        sleep(3);
        printf("%d hello from parent again\n", getpid());
        wait(NULL);
    }

    return 0;
}
