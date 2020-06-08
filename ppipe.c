#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "ppipe.h"

struct ppipe init_ppipe(size_t member_size) {
    struct ppipe p;
    pthread_mutex_init(&p.mutex, NULL);
    pthread_cond_init(&p.not_empty, NULL);
    pthread_cond_init(&p.not_full, NULL);
    if ((p.pipebuf = malloc(member_size * PIPEBUFSIZ)) == NULL) {
        fputs("Out of memory.", stderr);
    }
    for (size_t i=0; i<PIPEBUFSIZ; i++) {
        p.pipebuf[i]=0;
    }
    p.start=0;
    p.end=0;
    p.member_size = member_size;
    p.closed = false;
    return(p);
}

void free_ppipe(struct ppipe p) {
    free(p.pipebuf);
}

void ppipe_write(struct ppipe *p, const void *i, bool close) {
    pthread_mutex_lock(&(p->mutex));
    while (p->end >= PIPEBUFSIZ && p->end > p->start) {
        /*printf("write%ld: p->end >= PIPEBUFSIZ && p->end > p->start\n", pthread_self());*/
        pthread_cond_wait(&(p->not_full), &(p->mutex));
    }
    if (p->start >= PIPEBUFSIZ) {
        /*printf("write%ld: p->start >= PIPEBUFSIZ\n", pthread_self());*/
        p->start = 0;
        p->end = 0;
    }
    memcpy(&(p->pipebuf[(p->end * p->member_size)]), i, p->member_size);
    p->end++;
    p->closed = close;
    /*printf("write%ld: start: %ld; end: %ld; closed: %d\n", pthread_self(), p->start, p->end, p->closed);*/
    pthread_cond_broadcast(&(p->not_empty));
    pthread_mutex_unlock(&(p->mutex));
}

void ppipe_close(struct ppipe *p) {
    pthread_mutex_lock(&(p->mutex));
    p->closed = true;
    pthread_mutex_unlock(&(p->mutex));
}

void ppipe_read(struct ppipe *p, void *out, bool *closed) {
    pthread_mutex_lock(&(p->mutex));
    while (p->end <= p->start && !p->closed) {
        printf("read%ld: p->end <= p->start && !p->closed\n", pthread_self());
        pthread_cond_wait(&(p->not_empty), &(p->mutex));
    }
    memcpy(out, &(p->pipebuf[(p->start * p->member_size)]), p->member_size);
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
    printf("read%ld: start: %ld; end: %ld; closed: %d\n", pthread_self(), p->start, p->end, *closed);
    printf("ppipe_read: p->closed: %d; local_closed: %d; p->start: %ld; p->end: %ld\n", p->closed, *closed, p->start, p->end);
    pthread_mutex_unlock(&(p->mutex));
}

void *generate_nums(void *inptr) {
    struct int_generator *gen = (struct int_generator *) inptr;
    /*printf("gen%ld:\n", pthread_self());*/
    for (int i=gen->start; i<gen->end; i+=gen->step) {
        ppipe_write(gen->p, &i, false);
    }
    /*ppipe_write(gen->p, &err, true);*/
    ppipe_close(gen->p);
    pthread_exit(NULL);
}

void *multiply_nums(void *inptr) {
    int i = 0;
    int i_mult = 0;
    bool closed = false;
    struct int_multiplier *mult = (struct int_multiplier *) inptr;
    /*printf("mult%ld:\n", pthread_self());*/
    while (!closed) {
        ppipe_read(mult->p, &i, &closed);
        if (closed) {
            break;
        }
        i_mult = i * mult->factor;
        ppipe_write(mult->op, &i_mult, closed);
    }
    ppipe_close(mult->op);
    pthread_exit(NULL);
}

void *print_nums(void *inptr) {
    int i = 0;
    bool closed = false;
    struct ppipe *p = (struct ppipe *) inptr;
    /*printf("print%ld:\n", pthread_self());*/
    while (!closed) {
        /*printf("print is reading:\n");*/
        ppipe_read(p, &i, &closed);
        if (!closed) {
            /*printf("printing!\n");*/
            printf("%d\n", i);
        }
    }
    pthread_exit(NULL);
}


