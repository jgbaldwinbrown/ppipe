#include <stdio.h>
#include "circarr.h"
#include "indexed_ints.h"

size_t int_indexer(void *input) {
    int i = 0;
    size_t j = 0;
    memcpy(&i, input, sizeof(int));
    j=i;
    /*printf("index: %ld\n", j);*/
    return(j);
}

void test1() {
    printf("test1:\n");
    struct circarr c = init_circarr(5, sizeof(int), int_indexer);
    int out = 0;
    for (int i=0; i<10; i++) {
        printf("adding:\n");
        circarr_add(&c, &i);
        circarr_print(c);
    }
    for (int j=0; j<10; j++) {
        if (circarr_full(c, j)) {
            printf("popping:\n");
            circarr_pop(&c, &out);
            printf("%d\n", out);
        }
    }
    circarr_free(c);
}

void test2() {
    printf("test2:\n");
    struct circarr c = init_circarr(5, sizeof(int), int_indexer);
    int in[] = {5, 3, 4, 2, 6, 7, 1, 0, 9, 12, 10, 11, 8};
    int out = 0;
    for (size_t i=0; i<(sizeof(in) / sizeof(in[0])); i++) {
        printf("adding:\n");
        circarr_add(&c, &in[i]);
        circarr_print(c);
    }
    for (size_t j=0; j<(sizeof(in) / sizeof(in[0])); j++) {
        if (circarr_full(c, j)) {
            printf("popping:\n");
            circarr_pop(&c, &out);
            printf("%d\n", out);
        }
    }
    circarr_free(c);
}

void test3() {
    printf("test3:\n");
    struct circarr c = init_circarr(5, sizeof(int), int_indexer);
    int in[] = {5, 3, 4, 2, 6, 7, 1, 0, 9, 12, 10, 11, 8};
    int out = 0;
    for (size_t i=0; i<(sizeof(in) / sizeof(in[0])); i++) {
        printf("adding:\n");
        circarr_add(&c, &in[i]);
        /*circarr_print(c);*/
        while (circarr_full(c, c.pos)) {
            printf("popping:\n");
            circarr_pop(&c, &out);
            printf("%d\n", out);
        }
    }
    while (circarr_full(c, c.pos)) {
        printf("popping:\n");
        circarr_pop(&c, &out);
        printf("%d\n", out);
        /*circarr_print(c);*/
    }
    circarr_free(c);
}

void test4() {
    printf("test4:\n");
    struct circarr c = init_circarr(5, sizeof(struct indexed_int), index_indexed_int);
    struct indexed_int ii;
    struct indexed_int oo;
    for (size_t i=0; i<10; i++) {
        printf("adding:\n");
        ii.index = i;
        ii.value = i*100;
        circarr_add(&c, &ii);
        /*circarr_print(c);*/
        while (circarr_full(c, c.pos)) {
            printf("popping:\n");
            circarr_pop(&c, &oo);
            indexed_int_print(oo);
        }
    }
    while (circarr_full(c, c.pos)) {
        printf("popping:\n");
        circarr_pop(&c, &oo);
        indexed_int_print(oo);
    }
    circarr_free(c);
}

int main() {
    test1();
    test2();
    test3();
    test4();
    return(0);
}
