// https://godbolt.org/
// использовать x86-64 gcc 14.2 -O0 & -O1 & -O3

#include <stdio.h>
#include <stdlib.h>


int kek(int x) {
    if (__builtin_expect(x == 0, 0))  // unlikely
        return -1;
    return 0;
}

__attribute__((noreturn))
void fail() {
    __builtin_trap();
}

inline int add(int a, int b) {
    return a + b;
}

int use_inline() {
    return add(1, 2);
}

__attribute__((naked)) void handler() {
    __asm__("nop");
}

struct Normal {
    char  a;
    int   b;
};

struct Normal n;

int lel() {
    printf("sizeof(Normal) = %zu\n", sizeof(struct Normal));
    n.a = 'a';
    n.b = 1;
}

struct __attribute__((packed)) Packed {
    char  a;
    int   b;
};

struct Packed p;

int lelp() {
    printf("sizeof(Normal) = %zu\n", sizeof(struct Packed));
    p.a = 'a';
    p.b = 1;
}

int xa __attribute__((aligned(64)));

[[noreturn]]
void die(const char *msg) {
    exit(1);
}