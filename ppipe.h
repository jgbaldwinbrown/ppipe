#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

struct ppipe {
    pthread_mutex_t mutex;
    pthread_cond_t not_empty;
    pthread_cond_t not_full;
    int pipebuf[PIPEBUFSIZ];
    size_t start;
    size_t end;
    bool closed;
    size_t member_size;
    size_t nmemb;
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

struct ppipe init_ppipe();
void write_int_to_ppipe(struct ppipe *p, int i, bool close);
int read_int_from_ppipe(struct ppipe *p, bool *closed);
void *generate_nums(void *inptr);
void *multiply_nums(void *inptr);
void *print_nums(void *inptr);
