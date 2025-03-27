// gcc -g -Wall -Wextra -O2 cpu_atomic.c -o cpu_atomic -pthread
// ./cpu_atomic

// теперь раскомментируем /* __attribute__((__packed__)) */
// перекомпилируем и перезапустим программу и...
// поймём, что я вам наврал! Или всё-таки нет?

#include <stdio.h>
#include <stdint.h>
#include <pthread.h>
#include <unistd.h>

struct /* __attribute__((__packed__)) */ Shared {
    int32_t shit;
    int64_t value;
};

struct Shared shared_data;

void* writer_thread(void* arg) {
    (void)arg;
    int64_t a = 0xaaaaaaaabbbbbbbb;
    int64_t b = 0xccccccccdddddddd;

    while (1) {
        shared_data.value = a;
        usleep(100);
        shared_data.value = b;
        usleep(100);
    }
    return NULL;
}

void* reader_thread(void* arg) {
    (void)arg;
    while (1) {
        int64_t v = shared_data.value;
        printf("Read: 0x%016llx\n", (unsigned long long)v);
        usleep(50);
    }
    return NULL;
}

int main(void) {
    printf("Shared size: %zu\n", sizeof(shared_data));
    sleep(2);

    pthread_t writer, reader;

    pthread_create(&writer, NULL, writer_thread, NULL);
    pthread_create(&reader, NULL, reader_thread, NULL);

    pthread_join(writer, NULL);
    pthread_join(reader, NULL);

    return 0;
}
