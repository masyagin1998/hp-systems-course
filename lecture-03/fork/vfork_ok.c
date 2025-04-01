// gcc -std=c11 -O2 -Wall -Wextra -pthread vfork_ok.c -o vfork_ok
// ./vfork_ok

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>

int main() {
    printf("%d hello from parent\n", getpid());

    pid_t pid = vfork();
    if (pid == 0) {
        char buf[64];
        int len = snprintf(buf, sizeof(buf), "%d hello from child\n", getpid());
        char *args[] = {"echo", buf, NULL};
        execvp("echo", args);
        write(STDOUT_FILENO, buf, len);
        sleep(5);
    } else {
        printf("%d hello from parent again\n", getpid());
        wait(NULL);
    }

    return 0;
}
