#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "ppipe_merger.h"
#include "indexed_ints.h"

#define PIPEBUFSIZ	20

int main(int argc, char *argv[]) {
    pthread_t gen_thread;
    pthread_t mult_thread;
    pthread_t mult_thread2;
    pthread_t merge_thread;
    pthread_t print_thread;
    int rc;
    
    struct ppipe p = init_ppipe(sizeof(struct indexed_int));
    struct ppipe mp = init_ppipe(sizeof(struct indexed_int));
    struct ppipe op = init_ppipe(sizeof(struct indexed_int));
    
    struct int_generator gen;
    gen.start = 100;
    gen.end = 150;
    gen.step = 2;
    gen.p = &p;
    
    struct int_multiplier mult;
    mult.factor = 1000;
    mult.p = &p;
    mult.op = &mp;
    
    struct ppipe_merger merger;
    merger.p = &mp;
    merger.op = &op;
    merger.indexer = index_indexed_int;

    rc = pthread_create(&gen_thread, NULL, generate_indexed_nums, (void *)&gen);
    if (rc){
        printf("ERROR; return code from pthread_create() is %d\n", rc);
        exit(-1);
    }
    
    rc = pthread_create(&mult_thread, NULL, multiply_indexed_nums, (void *)&mult);
    if (rc){
        printf("ERROR; return code from pthread_create() is %d\n", rc);
        exit(-1);
    }
    
    rc = pthread_create(&mult_thread2, NULL, multiply_indexed_nums, (void *)&mult);
    if (rc){
        printf("ERROR; return code from pthread_create() is %d\n", rc);
        exit(-1);
    }
    
    rc = pthread_create(&merge_thread, NULL, ppipe_merge, (void *)&merger);
    if (rc){
        printf("ERROR; return code from pthread_create() is %d\n", rc);
        exit(-1);
    }

    rc = pthread_create(&print_thread, NULL, print_indexed_nums, (void *)&op);
    if (rc){
        printf("ERROR; return code from pthread_create() is %d\n", rc);
        exit(-1);
    }

    printf("made all threads\n");
 
    pthread_join(gen_thread, NULL);
    pthread_join(mult_thread, NULL);
    pthread_join(mult_thread2, NULL);
    pthread_join(merge_thread, NULL);
    pthread_join(print_thread, NULL);
    
    free_ppipe(p);
    free_ppipe(mp);
    free_ppipe(op);

    /* Last thing that main() should do */
    pthread_exit(NULL);
}
