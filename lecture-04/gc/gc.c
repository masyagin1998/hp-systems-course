// sudo apt install libgc-dev
// gcc -std=c11 -O2 -Wall -Wextra -o gc gc.c -lgc
// valgrind --leak-check=full --show-leak-kinds=all ./gc

// поменяем с GC_MALLOC на malloc, перекомпилируем и снова запустим под valgrind'ом

#include <stdio.h>
#include <gc/gc.h>
#include <stdlib.h>
#include <string.h>

#define NUM_OBJECTS 100000

int main(void) {
    GC_INIT();

    printf("Начинаем аллокацию %d объектов...\n", NUM_OBJECTS);

    for (int i = 0; i < NUM_OBJECTS; ++i) {
        int *p = (int *)GC_MALLOC(1024 * sizeof(int));
        // int *p = (int *)malloc(1024 * sizeof(int));
        *p = i;
        if (i % 20000 == 0) {
            printf("GC_malloc #%d: %p -> %d\n", i, (void *)p, *p);
        }
    }

    printf("Аллокация завершена. Запускаем сборку мусора...\n");

    GC_gcollect();

    printf("Готово. Выходим.\n");
    return 0;
}