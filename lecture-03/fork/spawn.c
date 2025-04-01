// gcc -std=c11 -O2 -Wall -Wextra -pthread spawn.c -o spawn
// ./spawn

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <spawn.h>
#include <stdlib.h>

extern char **environ;

int main() {
    printf("%d hello from parent\n", getpid());

    pid_t pid;
    char *argv[] = {"echo", "hello from child", NULL};

    posix_spawnp(&pid, "echo", NULL, NULL, argv, environ);

    sleep(3);
    printf("%d hello from parent again\n", getpid());

    waitpid(pid, NULL, 0);
    return 0;
}
