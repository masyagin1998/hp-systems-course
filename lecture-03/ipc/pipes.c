// gcc -std=c11 -O2 -Wall -Wextra -o pipes pipes.c
// ./pipes
// strace ./pipes
// strace -c -e write ./pipes

// [✓] Redo for buffered I/O 

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

#define BUF_SIZE 1024
#define ITERATIONS 5

void run_server(int read_fd, int write_fd) {
    char buffer[BUF_SIZE];

    for (int i = 0; i < ITERATIONS; ++i) {
        // Per buffer read
        ssize_t read_num = 0;
        read_num = read(read_fd, &buffer, sizeof(buffer)); 
        buffer[read_num] = '\0';

        /* Per character read (Very consuming & Lot of syscall per character) 
        every read/write function lead to system call to the kernel.
        so it's very bad to make read for every character need to be read 
        
         ssize_t pos = 0;
         char c;
         while (read(read_fd, &c, 1) == 1 && c != '\0') {
             if (pos < BUF_SIZE - 1) {
                 buffer[pos++] = c;
             }
         }
         buffer[pos] = '\0';
        
         */
        printf("%d got message \"%s\" from client\n", getpid(), buffer);

        write(write_fd, buffer, strlen(buffer));
        write(write_fd, "\0", 1);
    }
}

void run_client(int write_fd, int read_fd) {
    char buffer[BUF_SIZE];

    for (int i = 0; i < ITERATIONS; ++i) {
        snprintf(buffer, sizeof(buffer), "ping server %d", i);
        printf("%d sending message to server...\n", getpid());

        write(write_fd, buffer, strlen(buffer));
        write(write_fd, "\0", 1);

        // Buffer read
        ssize_t read_num = 0;
        read_num = read(read_fd, &buffer, sizeof(buffer));
        if (read_num > 0 && buffer[read_num-1] != '\0') {
            buffer[read_num] = '\0';
        } 

        /* Per character read (Very consuming & Lot of syscall per character) 
        every read/write function lead to system call to the kernel.
        so it's very bad to make read for every character need to be read 
        
        
        ssize_t pos = 0;
         char c;
         while (read(read_fd, &c, 1) == 1 && c != '\0') {
             if (pos < BUF_SIZE - 1) {
                 reply[pos++] = c;
             }
         }
         reply[pos] = '\0';
        
         */
        printf("%d got message \"%s\" from server\n", getpid(), buffer);
        sleep(1);
    }
}

int main() {
    int pipe_client_to_server[2];
    int pipe_server_to_client[2];

    pipe(pipe_client_to_server);
    pipe(pipe_server_to_client);

    pid_t pid = fork();

    if (pid == 0) {
        // child = server
        close(pipe_client_to_server[1]); // не пишем в client→server
        close(pipe_server_to_client[0]); // не читаем из server→client

        run_server(pipe_client_to_server[0], pipe_server_to_client[1]);

        close(pipe_client_to_server[0]);
        close(pipe_server_to_client[1]);
        _exit(0);
    } else {
        // parent = client
        close(pipe_client_to_server[0]); // не читаем из client→server
        close(pipe_server_to_client[1]); // не пишем в server→client

        run_client(pipe_client_to_server[1], pipe_server_to_client[0]);

        close(pipe_client_to_server[1]);
        close(pipe_server_to_client[0]);
        wait(NULL);
    }

    return 0;
}
