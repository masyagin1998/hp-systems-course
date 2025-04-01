// gcc -std=gnu11 -O2 -Wall -Wextra -o tokenizer tokenizer.c
// ./tokenizer

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdint.h>

typedef struct Coroutine {
    void* rsp;
    uint64_t regs[6]; // rbx, rbp, r12, r13, r14, r15
} Coroutine;

void switch_context(Coroutine *from, Coroutine *to);

#define STACK_SIZE  (1024 * 64)

Coroutine main_ctx;
Coroutine read_ctx;
Coroutine print_ctx;


char current_word[256];

const char *input_stream = "Hello world from coroutine subsystem!";

void read_coroutine() {
    const char *p = input_stream;
    while (*p != '\0') {
        while (*p == ' ') {
            p++;
        }
        if (*p == '\0') {
            break;
        }
        int i = 0;
        while (*p != ' ' && *p != '\0' && i < (int)(sizeof(current_word) - 1)) {
            current_word[i++] = *p;
            p++;
        }
        current_word[i] = '\0';
        switch_context(&read_ctx, &print_ctx);
    }

    switch_context(&read_ctx, &print_ctx);
    switch_context(&read_ctx, &main_ctx);
}

void printer_coroutine() {
    while (1) {
        if (current_word[0] != '\0') {
            printf("Printer: %s\n", current_word);
            current_word[0] = '\0';
        }
        switch_context(&print_ctx, &read_ctx);
    }
}

void coroutine_init(Coroutine *ctx, void (*func)()) {
    void *stack = malloc(STACK_SIZE);
    if (!stack) {
        perror("malloc");
        exit(1);
    }
    void *stack_top = (uint8_t*)stack + STACK_SIZE;

    uint64_t *sp = (uint64_t*)stack_top;
    // Push the coroutine function address on the new stack.
    *(--sp) = (uint64_t)func;

    ctx->rsp = sp;

    for (int i = 0; i < 6; ++i) {
        ctx->regs[i] = 0;
    }
}

__attribute__((noinline, naked))
void switch_context(Coroutine *from, Coroutine *to) {
    __asm__ volatile (
        "movq %%rbx, 0(%0)\n\t"
        "movq %%rbp, 8(%0)\n\t"
        "movq %%r12, 16(%0)\n\t"
        "movq %%r13, 24(%0)\n\t"
        "movq %%r14, 32(%0)\n\t"
        "movq %%r15, 40(%0)\n\t"
        "movq %%rsp, %1\n\t"

        "movq 0(%2), %%rbx\n\t"
        "movq 8(%2), %%rbp\n\t"
        "movq 16(%2), %%r12\n\t"
        "movq 24(%2), %%r13\n\t"
        "movq 32(%2), %%r14\n\t"
        "movq 40(%2), %%r15\n\t"
        "movq %3, %%rsp\n\t"
        "ret\n\t"
        :
        : "r" (from->regs), "m" (from->rsp),
          "r" (to->regs),   "m" (to->rsp)
        : "memory"
    );
}

int main() {
    coroutine_init(&read_ctx, read_coroutine);
    coroutine_init(&print_ctx, printer_coroutine);

    printf("Main: starting coroutines\n");
    switch_context(&main_ctx, &read_ctx);
    printf("Main: coroutines finished\n");
    return 0;
}
