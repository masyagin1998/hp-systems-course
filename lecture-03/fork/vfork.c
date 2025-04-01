// gcc -std=c11 -O2 -Wall -Wextra -pthread vfork.c -o vfork
// ./vfork

// *** stack smashing detected ***: terminated
// Aborted (core dumped)

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
        write(STDOUT_FILENO, buf, len);
        sleep(6);
    } else {
        sleep(3);
        printf("%d hello from parent again\n", getpid());
        wait(NULL);
    }

    return 0;
}
