#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define CIRCARRSIZ 20

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
}

size_t void_indexer(void *v) {
    struct indexed_void *i = (struct indexed_void *) v;
    return(i->index);
}

void double_circular_array(struct circarr *c) {
    unsigned char *newbuf;
    bool *newfull;
    size_t new_bufsiz = c->bufsiz * 2;
    
    if ((newbuf = calloc(new_bufsiz, c->member_size)) == NULL) {
        fputs("Out of memory.", stderr);
        exit(1);
    }
    
    if ((newfull = calloc(new_bufsiz, sizeof(bool))) == NULL) {
        fputs("Out of memory.", stderr);
        exit(1);
    }
    
    for (size_t i=c->pos; i<c->bufsiz; i++) {
        size_t i_oldmod = i % c->bufsiz;
        size_t i_mod = i % new_bufsiz;
        memcpy(&newbuf[i_mod * c->member_size], c->buf[i_oldmod * c->member_size], c->member_size);
    }
    for (size_t i=c->pos; i<c->bufsiz; i++) {
        size_t i_oldmod = i % c->bufsiz;
        size_t i_mod = i % new_bufsiz;
        newfull[i_mod] = c->full[i_oldmod];
    }
    
    c->bufsiz = new_bufsiz;
    
    free(c->buf);
    free(c->full);
    c->buf = newbuf;
    c->full = newfull;
}

struct circarr init_circular_array(size_t bufsiz, size_t member_size, size_t (*indexer) (void *)) {
    struct circarr c;
    if ((c->buf = calloc(bufsiz, member_size)) == NULL) {
        fputs("Out of memory.", stderr);
    }
    if ((c->full = calloc(bufsiz, sizeof(bool))) == NULL) {
        fputs("Out of memory.", stderr);
    }
    c->pos = 0;
    c->member_size = member_size;
    c->bufsiz = bufsiz;
    c->index = indexer;
    return(c);
}

void circular_array_add(struct circarr c, void *value) {
    size_t index = c.index(value);
    if 
}

void circular_array_pop(struct circarr c, void *out) {
    memcpy(out, &(c->buf[c->bufpos * c->member_size], c->member_size);
    c->full[c->pos % c->bufsiz] = false;
    c->pos++;
}

