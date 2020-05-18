#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#define PIPEBUFSIZ	20

struct ppipe {
    pthread_mutex_t mutex;
    pthread_cond_t not_empty;
    pthread_cond_t not_full;
    int pipebuf[PIPEBUFSIZ];
    size_t start;
    size_t end;
    bool closed;
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

struct indexed_int {
    size_t index;
    int value;
};

struct indexed_int_holder {
    struct indexed_int indexed_int;
    bool filled;
};

struct thread_merger {
    struct ppipe *p;
    struct ppipe *op;
    size_t (*indexer) (struct indexed_int);
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

size_t int_indexer(struct indexed_int i) {
    return(i.index);
}

struct indexed_int_holder *double_circular_array(struct indexed_int_holder *buf, size_t *bufsiz, size_t bufpos) {
    static struct indexed_int_holder empty_holder;
    empty_holder.indexed_int.index = 0;
    empty_holder.indexed_int.value = 0;
    empty_holder.filled = false;
    
    if ((buf = realloc(buf, sizeof(struct indexed_int_holder) * (*bufsiz) * 2)) == NULL) {
        fputs("Out of memory.", stderr);
        exit(1);
    }
    for (size_t loop_i=*bufsiz; loop_i<*bufsiz; loop_i++) {
        buf[loop_i] = empty_holder;
    }
    for (size_t i=0; i<bufpos; i++) {
        buf[i+(*bufsiz)] = buf[i];
        buf[i] = empty_holder;
    }
    *bufsiz*=2;
    return(buf);
}

void *merge_threads(void *inptr) {
    struct thread_merger *merger = (struct thread_merger *) inptr;
    bool closed = false;
    struct indexed_int i;
    size_t curindex = 0;
    size_t pos_to_edit = 0;
    size_t bufsiz = PIPEBUFSIZ;
    struct indexed_int_holder *buf;
    if ((buf = malloc(sizeof(struct indexed_int_holder) * bufsiz)) == NULL) {
        fputs("Out of memory.", stderr);
        exit(1);
    }
    for (size_t loop_i=0; loop_i<bufsiz; loop_i++) {
        buf[loop_i].indexed_int.index = 0;
        buf[loop_i].indexed_int.value = 0;
        buf[loop_i].filled = false;
    }
    size_t curbufpos = 0;
    
    while(!closed) {
        i = read_int_from_ppipe(merger->p, &closed);
        if merger->indexer(i) <= curindex {
            write_int_to_ppipe(merger->p, i.value, closed);
            buf[curbufpos].filled = false;
            curbufpos = (curbufpos+1) % bufsiz;
            curindex++;
        } else {
            while (merger->indexer(i) - curindex > bufsiz) {
                buf = double_circular_array(buf, &bufsiz, curbufpos);
            }
            pos_to_edit = (curbufpos + (merger->indexer(i) - curindex))
            buf[pos_to_edit].indexed_int = i;
            buf[pos_to_edit].full = true;
        }
        while (merger->indexer(buf[curpos].indexed_int) == curindex + 1) {
            if (closed && !buf[curbufpos].full) {
                write_int_to_ppipe(merger->p, buf[curpos].indexed_int.value, closed);
            } else {
                write_int_to_ppipe(merger->p, buf[curpos].indexed_int.value, false);
            }
            curindex++;
            curbufpos = (curbufpos+1) & bufsiz;
        }
    }
    
    while (merger->indexer(buf[curpos].indexed_int) == curindex + 1) {
        if (closed && !buf[curbufpos].full) {
            write_int_to_ppipe(merger->p, buf[curpos].indexed_int.value, closed);
        } else {
            write_int_to_ppipe(merger->p, buf[curpos].indexed_int.value, false);
        }
        curindex++;
        curbufpos = (curbufpos+1) & bufsiz;
    }
    
    free(buf);
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


int main(int argc, char *argv[]) {
    pthread_t gen_thread;
    pthread_t mult_thread;
    pthread_t mult_thread2;
    pthread_t print_thread;
    pthread_t print_thread2;
    int rc;
    
    struct ppipe p = init_ppipe();
    struct ppipe op = init_ppipe();
    
    struct int_generator gen;
    gen.start = 0;
    gen.end = 100;
    gen.step = 2;
    gen.p = &p;
    
    struct int_multiplier mult;
    mult.factor = 1000;
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
    
    rc = pthread_create(&mult_thread2, NULL, multiply_nums, (void *)&mult);
    if (rc){
        printf("ERROR; return code from pthread_create() is %d\n", rc);
        exit(-1);
    }
    
    rc = pthread_create(&print_thread, NULL, print_nums, (void *)&op);
    if (rc){
        printf("ERROR; return code from pthread_create() is %d\n", rc);
        exit(-1);
    }

    rc = pthread_create(&print_thread2, NULL, print_nums, (void *)&op);
    if (rc){
        printf("ERROR; return code from pthread_create() is %d\n", rc);
        exit(-1);
    }
    printf("made all threads\n");
 
    pthread_join(gen_thread, NULL);
    pthread_join(mult_thread, NULL);
    pthread_join(print_thread, NULL);
    pthread_join(mult_thread2, NULL);
    pthread_join(print_thread2, NULL);

    /* Last thing that main() should do */
    pthread_exit(NULL);
}
