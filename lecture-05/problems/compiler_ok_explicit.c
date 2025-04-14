// https://godbolt.org/
// использовать x86-64 gcc 9.5 -O3 & x86-64 gcc 14.2 -O3 &
// а ещё можно использовать -masm=att...
// а ещё и ARM 64 GCC 9.5.0 -O3 & ARM 64 GCC 14.2.0 -O3...

#include <stdatomic.h>

atomic_int a = 0;
atomic_int b = 0;

void dummy() {
    atomic_store_explicit(&a, 1, memory_order_seq_cst);
    atomic_store_explicit(&b, 2, memory_order_seq_cst);
}
