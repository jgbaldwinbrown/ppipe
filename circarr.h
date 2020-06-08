#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#ifndef CIRCARR_H
#define CIRCARR_H

struct indexed_void {
    size_t index;
    void *value;
};

struct circarr {
    unsigned char *buf;
    bool *full;
    size_t pos;
    size_t member_size;
    size_t bufsiz;
    size_t (*index) (void *);
};

#endif

size_t void_indexer(void *v);
void double_circular_array(struct circarr *c);
struct circarr init_circarr(size_t bufsiz, size_t member_size, size_t (*indexer) (void *));
void circarr_add(struct circarr *c, void *value);
void circarr_pop(struct circarr *c, void *out);
bool circarr_full(struct circarr c, size_t index);
bool circarr_poppable(struct circarr c);
void circarr_print(struct circarr c);
void circarr_free(struct circarr c);
