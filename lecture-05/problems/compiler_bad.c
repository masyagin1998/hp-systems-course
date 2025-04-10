// https://godbolt.org/
// использовать x86-64 gcc 9.5 -O3 & x86-64 gcc 14.2 -O3
// а ещё можно использовать -masm=att...
// а ещё и ARM 64 GCC 9.5.0 -O3 & ARM 64 GCC 14.2.0 -O3...

#include <stdatomic.h>

int a = 0;
int b = 0;

void dummy() {
    a = 1;
    atomic_thread_fence(memory_order_seq_cst);
    b = 2;
    atomic_thread_fence(memory_order_seq_cst);
}
