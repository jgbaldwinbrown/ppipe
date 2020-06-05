#include "circarr.h"

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
        memcpy(&newbuf[i_mod * c->member_size], &c->buf[i_oldmod * c->member_size], c->member_size);
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

struct circarr init_circarr(size_t bufsiz, size_t member_size, size_t (*indexer) (void *)) {
    struct circarr c;
    if ((c.buf = calloc(bufsiz, member_size)) == NULL) {
        fputs("Out of memory.", stderr);
    }
    for (size_t i=0; i<(bufsiz*member_size); i++) {
        c.buf[i] = 0;
    }
    if ((c.full = calloc(bufsiz, sizeof(bool))) == NULL) {
        fputs("Out of memory.", stderr);
    }
    c.pos = 0;
    c.member_size = member_size;
    c.bufsiz = bufsiz;
    c.index = indexer;
    circarr_print(c);
    return(c);
}

void circarr_add(struct circarr *c, void *value) {
    size_t index = c->index(value);
    size_t writepos = 0;
    if ((index - c->pos) >= c->bufsiz) {
        double_circular_array(c);
    }
    writepos = (index % c->bufsiz) * c->member_size;
    memcpy(&(c->buf[writepos]), value, c->member_size);
    c->full[index%c->bufsiz] = true;
}

void circarr_pop(struct circarr *c, void *out) {
    size_t readpos = (c->pos % c->bufsiz) * c->member_size;
    memcpy(out, &(c->buf[readpos]), c->member_size);
    c->full[c->pos % c->bufsiz] = false;
    c->pos++;
}

bool circarr_full(struct circarr c, size_t index) {
    if (index - c.pos > c.bufsiz) {
        return(false);
    } else {
        return(c.full[index % c.bufsiz]);
    }
}

bool circarr_poppable(struct circarr c) {
    return(c.full[c.pos % c.bufsiz]);
}

void circarr_print(struct circarr c) {
    int temp = 0;
    size_t pos = 0;
    for (size_t i=0; i < c.bufsiz; i++) {
        pos = i * c.member_size;
        memcpy(&temp, &(c.buf[pos]), c.member_size);
        printf("\t%d", temp);
    }
    printf("\n");
    for (size_t i=0; i < c.bufsiz; i++) {
        printf("\t%d", c.full[i]);
    }
    printf("\n");
}

void circarr_free(struct circarr c) {
    free(c.buf);
    free(c.full);
}
