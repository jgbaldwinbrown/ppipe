#include "indexed_ints.h"
#include "ppipe.h"

void *generate_indexed_nums(void *inptr) {
    int err=-1;
    struct indexed_int ii;
    size_t counter = 0;
    struct int_generator *gen = (struct int_generator *) inptr;
    /*printf("gen%ld:\n", pthread_self());*/
    for (int i=gen->start; i<gen->end; i+=gen->step) {
        ii.index = counter;
        ii.value = i;
        printf("generate: ");
        indexed_int_print(ii);
        ppipe_write(gen->p, &ii, false);
        counter++;
    }
    ppipe_close(gen->p);
    pthread_exit(NULL);
}

void *multiply_indexed_nums(void *inptr) {
    struct indexed_int ii;
    int i = 0;
    int i_mult = 0;
    bool closed = false;
    struct int_multiplier *mult = (struct int_multiplier *) inptr;
    /*printf("mult%ld:\n", pthread_self());*/
    while (!closed) {
        ppipe_read(mult->p, &ii, &closed);
        if (closed) {
            break;
        }
        i = ii.value;
        i_mult = i * mult->factor;
        ii.value = i_mult;
        printf("multiply: ");
        indexed_int_print(ii);
        ppipe_write(mult->op, &(ii.value), closed);
    }
    ppipe_close(mult->op);
    pthread_exit(NULL);
}

void *print_indexed_nums(void *inptr) {
    struct indexed_int ii;
    bool closed = false;
    struct ppipe *p = (struct ppipe *) inptr;
    /*printf("print%ld:\n", pthread_self());*/
    while (!closed) {
        /*printf("print is reading:\n");*/
        ppipe_read(p, &ii, &closed);
        if (!closed) {
            /*printf("printing!\n");*/
            indexed_int_print(ii);
        }
    }
    pthread_exit(NULL);
}

size_t index_indexed_int(void *inptr) {
    struct indexed_int *ii = inptr;
    size_t jj;
    memcpy(&jj, &(ii->index), sizeof(jj));
    return(jj);
}

void indexed_int_print(struct indexed_int ii) {
    printf("index: %10ld; value: %10d\n", ii.index, ii.value);
}
