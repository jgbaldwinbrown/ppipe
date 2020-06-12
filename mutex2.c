#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#define PIPEBUFSIZ	20

struct ppipe {
    pthread_mutex_t mutex;
    pthread_cond_t not_empty;
    pthread_cond_t not_full;
    int pipebuf[PIPEBUFSIZ];
    size_t start;
    size_t end;
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
    return(p);
}

void write_int_to_ppipe(struct ppipe *p, int i) {
    pthread_mutex_lock(&(p->mutex));
    if (p->end >= PIPEBUFSIZ && p->end > p->start) {
        pthread_cond_wait(&(p->not_full), &(p->mutex));
    }
    if (p->start >= PIPEBUFSIZ) {
        p->start = 0;
        p->end = 0;
    }
    p->pipebuf[p->end] = i;
    p->end++;
    /*printf("write: start: %ld; end: %ld; val: %d\n", p->start, p->end, i);*/
    pthread_cond_signal(&(p->not_empty));
    pthread_mutex_unlock(&(p->mutex));
}

int read_int_from_ppipe(struct ppipe *p) {
    int i;
    pthread_mutex_lock(&(p->mutex));
    if (p->end <= p->start) {
        pthread_cond_wait(&(p->not_empty), &(p->mutex));
    }
    i = p->pipebuf[p->start];
    p->start++;
    if (p->end < PIPEBUFSIZ || p->end <= p->start) {
        pthread_cond_signal(&(p->not_full));
    }
    /*printf("read: start: %ld; end: %ld; val: %d\n", p->start, p->end, i);*/
    pthread_mutex_unlock(&(p->mutex));
    return(i);
}

void *generate_nums(void *inptr) {
    struct int_generator *gen = (struct int_generator *) inptr;
    for (int i=gen->start; i<gen->end; i+=gen->step) {
        write_int_to_ppipe(gen->p, i);
    }
    write_int_to_ppipe(gen->p, -1);
    pthread_exit(NULL);
}

void *multiply_nums(void *inptr) {
    int i = 0;
    int i_mult = 0;
    struct int_multiplier *mult = (struct int_multiplier *) inptr;
    while (i != -1) {
        i = read_int_from_ppipe(mult->p);
        i_mult = i * mult->factor;
        write_int_to_ppipe(mult->op, i_mult);
    }
    pthread_exit(NULL);
}

void *print_nums(void *inptr) {
    int i = 0;
    struct ppipe *p = (struct ppipe *) inptr;
    while (i >= 0) {
        i = read_int_from_ppipe(p);
        printf("%d\n", i);
    }
    pthread_exit(NULL);
}


int main(int argc, char *argv[]) {
    pthread_t gen_thread;
    pthread_t mult_thread;
    pthread_t print_thread;
    int rc;
    
    struct ppipe p = init_ppipe();
    struct ppipe op = init_ppipe();
    
    struct int_generator gen;
    gen.start = 0;
    gen.end = 100;
    gen.step = 2;
    gen.p = &p;
    
    struct int_multiplier mult;
    mult.factor = 2;
    mult.p = &p;
    mult.op = &op;

    rc = pthread_create(&gen_thread, NULL, generate_nums, (void *)&gen);
    if (rc){
        printf("ERROR; return code from pthread_create() is %d\n", rc);
        exit(-1);
    }
    
    rc = pthread_create(&mult_thread, NULL, multiply_nums, (void *)&mult);
    if (rc){
        printf("ERROR; return code from pthread_create() is %d\n", rc);
        exit(-1);
    }
    
    rc = pthread_create(&print_thread, NULL, print_nums, (void *)&op);
    if (rc){
        printf("ERROR; return code from pthread_create() is %d\n", rc);
        exit(-1);
    }
    printf("made all threads\n");
 
    pthread_join(gen_thread, NULL);
    pthread_join(mult_thread, NULL);
    pthread_join(print_thread, NULL);

    /* Last thing that main() should do */
    pthread_exit(NULL);
}
