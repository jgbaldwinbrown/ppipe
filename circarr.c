#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define CIRCARRSIZ 20

struct indexed_void {
    size_t index;
    void *value;
};

struct indexed_void_holder {
    struct indexed_void indexed_void;
    bool filled;
};

struct circarr {
    unsigned char *buf;
    size_t pos;
    size_t member_size;
    size_t bufsiz;
}

struct thread_merger {
    struct ppipe *p;
    struct ppipe *op;
    size_t (*indexer) (struct indexed_void);
};

size_t void_indexer(struct indexed_void i) {
    return(i.index);
}

void double_circular_array(struct circarr *c) {
    if ((c->buf = realloc(c->buf, c->member_size * c->bufsiz * 2)) == NULL) {
        fputs("Out of memory.", stderr);
        exit(1);
    }
    for (size_t loop_i=c->bufsiz * c->member_size; loop_i< c->bufsiz * c->member_size * 2; loop_i++) {
        c->buf[loop_i] = 0;
    }
    for (size_t i=0; i<(c->bufpos * c->member_size); i++) {
        c->buf[(i+ (c->bufsiz * c->member_size))] = c->buf[i];
        c->buf[i] = 0;
    }
    c->bufsiz *= 2;
}

struct circarr init_circular_array(size_t bufsiz, size_t member_size) {
    struct circarr c;
    if ((c->buf = calloc(bufsiz, member_size)) == NULL) {
        fputs("Out of memory.", stderr);
    }
    c->pos = 0;
    c->member_size = member_size;
    c->bufsiz = bufsiz;
    return(c);
}

void circular_array_add(struct circarr c, void *value, size_t relative_index) {
    /*...*/
}

void circular_array_pop(struct circarr c, void *out) {
    memcpy(out, &(c->buf[c->bufpos * c->member_size], c->member_size);
    c->buf[c->bufpos].filled = false;
    c->bufpos = ((c->bufpos)+1 % c->bufsiz);
}

void *merge_threads(void *inptr) {
    struct thread_merger *merger = (struct thread_merger *) inptr;
    bool closed = false;
    struct indexed_void i;
    struct indexed_void_holder ih;
    size_t pos_to_edit = 0;
    size_t curindex = 0;
    struct circular_array c = init_circular_array(CIRCARRSIZ, sizeof(indexed_void_holder));
    
    while(!closed) {
        i = read_int_from_ppipe(merger->p, &closed);
        if merger->indexer(i) <= curindex {
            write_int_to_ppipe(merger->p, i.value, closed);
            circular_array_pop(c, &ih);
            curindex++;
        } else {
            /* in progress */
            while (merger->indexer(i) - curindex > bufsiz) {
                double_circular_array(c);
            }
            pos_to_edit = (curbufpos + (merger->indexer(i) - curindex))
            buf[pos_to_edit].indexed_void = i;
            buf[pos_to_edit].full = true;
        }
        while (merger->indexer(buf[curpos].indexed_void) == curindex + 1) {
            if (closed && !buf[curbufpos].full) {
                write_int_to_ppipe(merger->p, buf[curpos].indexed_void.value, closed);
            } else {
                write_int_to_ppipe(merger->p, buf[curpos].indexed_void.value, false);
            }
            curindex++;
            curbufpos = (curbufpos+1) & bufsiz;
        }
    }
    
    while (merger->indexer(buf[curpos].indexed_void) == curindex + 1) {
        if (closed && !buf[curbufpos].full) {
            write_int_to_ppipe(merger->p, buf[curpos].indexed_void.value, closed);
        } else {
            write_int_to_ppipe(merger->p, buf[curpos].indexed_void.value, false);
        }
        curindex++;
        curbufpos = (curbufpos+1) & bufsiz;
    }
    
    free(buf);
    pthread_exit(NULL);
}

