#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define PIPEBUFSIZ 20

#ifndef PPIPE_H
#define PPIPE_H
struct ppipe {
    pthread_mutex_t mutex;
    pthread_cond_t not_empty;
    pthread_cond_t not_full;
    unsigned char *pipebuf;
    size_t start;
    size_t end;
    bool closed;
    size_t member_size;
};

struct int_generator {
    int start;
    int end;
    int step;
    struct ppipe *p;
};

struct int_multiplier {
    int factor;
    struct ppipe *p;
    struct ppipe *op;
};

#endif

struct ppipe init_ppipe(size_t member_size);
void free_ppipe(struct ppipe p);
void ppipe_write(struct ppipe *p, const void *i, bool close);
void ppipe_close(struct ppipe *p);
void ppipe_read(struct ppipe *p, void *i, bool *closed);
void *generate_nums(void *inptr);
void *multiply_nums(void *inptr);
void *print_nums(void *inptr);
void ppipe_print_contents(struct ppipe *p, void (*print_func) (void *));
