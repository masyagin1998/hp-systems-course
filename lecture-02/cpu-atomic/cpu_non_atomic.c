// gcc -g -Wall -Wextra -O2 cpu_non_atomic.c -o cpu_non_atomic -pthread
// ./cpu_non_atomic

#include <stdio.h>
#include <stdint.h>
#include <pthread.h>
#include <unistd.h>

// Отступ в 60 байт, чтобы поле value начиналось на смещении 60
// и занимало байты с 60 по 67, то есть 4 байта окажутся в одной кэш-линии, а 4 — в следующей.
struct __attribute__((__packed__)) Shared {
    char shit[60];
    volatile int64_t value;
} __attribute__((aligned(64))); // принудительное выравнивание структуры по 64 байтам

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

        if ((v == (int64_t)0xaaaaaaaabbbbbbbbULL) ||
            (v == (int64_t)0xccccccccddddddddULL)) {
            continue;
            } else {
                printf("⚠️ Possibly torn value detected!\n");
                sleep(2);
            }
    }
    return NULL;
}

int main(void) {
    printf("Address of value: %p (offset mod 64 = %lu)\n",
           (void*)&shared_data.value,
           (uintptr_t)&shared_data.value % 64);
    printf("Shared size: %zu\n", sizeof(shared_data));
    sleep(2);

    pthread_t writer, reader;
    pthread_create(&writer, NULL, writer_thread, NULL);
    pthread_create(&reader, NULL, reader_thread, NULL);
    pthread_join(writer, NULL);
    pthread_join(reader, NULL);

    return 0;
}
