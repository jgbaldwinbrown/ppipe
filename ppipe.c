#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "ppipe.h"

struct ppipe init_ppipe(size_t member_size, size_t number_writers) {
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
    p.number_closed = 0;
    p.number_writers = number_writers;
    return(p);
}

void free_ppipe(struct ppipe p) {
    free(p.pipebuf);
}

void ppipe_write(struct ppipe *p, const void *i) {
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
    /*printf("write%ld: start: %ld; end: %ld; closed: %d\n", pthread_self(), p->start, p->end, p->closed);*/
    pthread_cond_broadcast(&(p->not_empty));
    pthread_mutex_unlock(&(p->mutex));
}

void ppipe_close(struct ppipe *p) {
    pthread_mutex_lock(&(p->mutex));
    (p->number_closed)++;
    pthread_cond_broadcast(&(p->not_empty));
    pthread_mutex_unlock(&(p->mutex));
}

void ppipe_read(struct ppipe *p, void *out, bool *closed) {
    pthread_mutex_lock(&(p->mutex));
    *closed = false;
    if ((p->end <= p->start) && (p->number_closed >= p->number_writers)) {
        *closed = true;
    } else {
        while (p->end <= p->start && (p->number_closed < p->number_writers)) {
            pthread_cond_wait(&(p->not_empty), &(p->mutex));
        }
        memcpy(out, &(p->pipebuf[(p->start * p->member_size)]), p->member_size);
        p->start++;
        if (p->end < PIPEBUFSIZ || p->end <= p->start) {
            pthread_cond_broadcast(&(p->not_full));
        }
    }
    pthread_mutex_unlock(&(p->mutex));
}

void *generate_nums(void *inptr) {
    struct int_generator *gen = (struct int_generator *) inptr;
    for (int i=gen->start; i<gen->end; i+=gen->step) {
        ppipe_write(gen->p, &i);
    }
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
        ppipe_write(mult->op, &i_mult);
    }
    ppipe_close(mult->op);
    pthread_exit(NULL);
}

void print_int(void *in) {
    int i=0;
    memcpy(&i, in, sizeof(int));
    printf("%d", i);
}


void *print_nums(void *inptr) {
    int i = 0;
    bool closed = false;
    struct ppipe *p = (struct ppipe *) inptr;
    while (!closed) {
        ppipe_read(p, &i, &closed);
        if (!closed) {
            printf("%d\n", i);
        }
    }
    pthread_exit(NULL);
}

void ppipe_print_contents(struct ppipe *p, void (*print_func) (void *)) {
    size_t pos_to_print = 0;
    pthread_mutex_lock(&(p->mutex));
    for (size_t i=p->start; i<p->end; i++) {
        pos_to_print = i * p->member_size;
        printf("\t");
        print_func(&(p->pipebuf[pos_to_print]));
    }
    pthread_mutex_unlock(&(p->mutex));
    printf("\n");
}

void *tee(void *inptr) {
    bool closed = false;
    struct teer *ateer = (struct teer *) inptr;
    unsigned char *buf = NULL;
    if ((buf = calloc(1, ateer->p->member_size)) == NULL) {
        fputs("Out of memory.", stderr);
        exit(1);
    }
    ppipe_read(ateer->p, buf, &closed);
    while (!closed) {
        ppipe_write(ateer->op1, buf);
        ppipe_write(ateer->op2, buf);
        ppipe_read(ateer->p, buf, &closed);
    }
    ppipe_close(ateer->op1);
    ppipe_close(ateer->op2);
    free(buf);
    pthread_exit(NULL);
}

