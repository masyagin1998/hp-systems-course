// gcc -std=gnu11 -O2 -Wall -Wextra -o coro coro.c
// ./coro

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

typedef struct Coroutine {
    void* rsp;
    uint64_t regs[6]; // rbx, rbp, r12, r13, r14, r15
} Coroutine;

void switch_context(Coroutine *from, Coroutine *to);

#define STACK_SIZE  (1024 * 64)

Coroutine main_ctx;
Coroutine coro_ctx;

void coroutine_entry() {
    printf("Coroutine: step 1\n");
    switch_context(&coro_ctx, &main_ctx);
    printf("Coroutine: step 2\n");
    switch_context(&coro_ctx, &main_ctx);
    printf("Coroutine: end\n");
    switch_context(&coro_ctx, &main_ctx);
}

void coroutine_init(Coroutine *ctx, void (*func)()) {
    void* stack = malloc(STACK_SIZE);
    void* stack_top = (uint8_t*)stack + STACK_SIZE;

    uint64_t *sp = (uint64_t*)stack_top;
    *(--sp) = (uint64_t)func;

    ctx->rsp = sp;

    for (int i = 0; i < 6; ++i) {
        ctx->regs[i] = 0;
    }
}

__attribute__((noinline, naked))
void switch_context(Coroutine *from, Coroutine *to) {
    asm volatile (
        // save callee-saved regs into from->regs
        "movq %%rbx, 0(%0)\n\t"
        "movq %%rbp, 8(%0)\n\t"
        "movq %%r12, 16(%0)\n\t"
        "movq %%r13, 24(%0)\n\t"
        "movq %%r14, 32(%0)\n\t"
        "movq %%r15, 40(%0)\n\t"
        "movq %%rsp, %1\n\t"

        // load to->regs
        "movq 0(%2), %%rbx\n\t"
        "movq 8(%2), %%rbp\n\t"
        "movq 16(%2), %%r12\n\t"
        "movq 24(%2), %%r13\n\t"
        "movq 32(%2), %%r14\n\t"
        "movq 40(%2), %%r15\n\t"
        "movq %3, %%rsp\n\t"

        // ret into new context
        "ret\n\t"
        :
        : "r" (from->regs), "m" (from->rsp),
          "r" (to->regs),   "m" (to->rsp)
        : "memory"
    );
}

int main() {
    coroutine_init(&coro_ctx, coroutine_entry);

    printf("Main: switching to coroutine\n");
    switch_context(&main_ctx, &coro_ctx);

    printf("Main: back from coroutine 1\n");
    switch_context(&main_ctx, &coro_ctx);

    printf("Main: back from coroutine 2\n");
    switch_context(&main_ctx, &coro_ctx);

    printf("Main: done\n");
    return 0;
}
