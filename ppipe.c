#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "ppipe.h"

struct ppipe init_ppipe() {
    struct ppipe p;
    pthread_mutex_init(&p.mutex, NULL);
    pthread_cond_init(&p.not_empty, NULL);
    pthread_cond_init(&p.not_full, NULL);
    for (size_t i=0; i<PIPEBUFSIZ; i++) {
        p.pipebuf[i]=0;
    }
    p.start=0;
    p.end=0;
    p.closed = false;
    return(p);
}

void write_int_to_ppipe(struct ppipe *p, int i, bool close) {
    pthread_mutex_lock(&(p->mutex));
    while (p->end >= PIPEBUFSIZ && p->end > p->start) {
        printf("write%ld: p->end >= PIPEBUFSIZ && p->end > p->start\n", pthread_self());
        pthread_cond_wait(&(p->not_full), &(p->mutex));
    }
    if (p->start >= PIPEBUFSIZ) {
        printf("write%ld: p->start >= PIPEBUFSIZ\n", pthread_self());
        p->start = 0;
        p->end = 0;
    }
    p->pipebuf[p->end] = i;
    p->end++;
    p->closed = close;
    printf("write%ld: start: %ld; end: %ld; val: %d; closed: %d\n", pthread_self(), p->start, p->end, i, p->closed);
    pthread_cond_broadcast(&(p->not_empty));
    pthread_mutex_unlock(&(p->mutex));
}

int read_int_from_ppipe(struct ppipe *p, bool *closed) {
    int i;
    pthread_mutex_lock(&(p->mutex));
    while (p->end <= p->start && !p->closed) {
        printf("read%ld: p->end <= p->start && !p->closed\n", pthread_self());
        pthread_cond_wait(&(p->not_empty), &(p->mutex));
    }
    i = p->pipebuf[p->start];
    p->start++;
    if (p->end < PIPEBUFSIZ || p->end <= p->start) {
        printf("read%ld: p->end < PIPEBUFSIZ || p->end <= p->start\n", pthread_self());
        pthread_cond_broadcast(&(p->not_full));
    }
    *closed = false;
    if ((p->end <= p->start) && p->closed) {
        printf("read%ld: (p->end <= p->start) && p->closed\n", pthread_self());
        *closed = p->closed;
    }
    printf("read%ld: start: %ld; end: %ld; val: %d; closed: %d\n", pthread_self(), p->start, p->end, i, *closed);
    pthread_mutex_unlock(&(p->mutex));
    return(i);
}

void *generate_nums(void *inptr) {
    struct int_generator *gen = (struct int_generator *) inptr;
    printf("gen%ld:\n", pthread_self());
    for (int i=gen->start; i<gen->end; i+=gen->step) {
        write_int_to_ppipe(gen->p, i, false);
    }
    write_int_to_ppipe(gen->p, -1, true);
    pthread_exit(NULL);
}

void *multiply_nums(void *inptr) {
    int i = 0;
    int i_mult = 0;
    bool closed = false;
    struct int_multiplier *mult = (struct int_multiplier *) inptr;
    printf("mult%ld:\n", pthread_self());
    while (!closed) {
        i = read_int_from_ppipe(mult->p, &closed);
        i_mult = i * mult->factor;
        write_int_to_ppipe(mult->op, i_mult, closed);
    }
    pthread_exit(NULL);
}

void *print_nums(void *inptr) {
    int i = 0;
    bool closed = false;
    struct ppipe *p = (struct ppipe *) inptr;
    printf("print%ld:\n", pthread_self());
    while (!closed) {
        i = read_int_from_ppipe(p, &closed);
        if (!closed) {
            printf("%d\n", i);
        }
    }
    pthread_exit(NULL);
}


