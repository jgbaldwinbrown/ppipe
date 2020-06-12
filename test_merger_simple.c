#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "ppipe_merger.h"
#include "indexed_ints.h"

#define PIPEBUFSIZ	20

size_t int_indexer(void *i) {
    int ii = 0;
    memcpy(&ii, i, sizeof(int));
    /*
    memcpy(&j, i, sizeof(int));
    */
    return(ii);
}

int main(int argc, char *argv[]) {
    pthread_t gen_thread;
    pthread_t merge_thread;
    pthread_t print_thread;
    int rc;
    
    struct ppipe p = init_ppipe(sizeof(int));
    struct ppipe op = init_ppipe(sizeof(int));
    
    struct int_generator gen;
    gen.start = 0;
    gen.end = 100;
    gen.step = 1;
    gen.p = &p;
    
    struct ppipe_merger merger;
    merger.p = &p;
    merger.op = &op;
    merger.indexer = int_indexer;

    rc = pthread_create(&gen_thread, NULL, generate_nums, (void *)&gen);
    if (rc){
        printf("ERROR; return code from pthread_create() is %d\n", rc);
        exit(-1);
    }
    
    rc = pthread_create(&merge_thread, NULL, ppipe_merge, (void *)&merger);
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
    pthread_join(merge_thread, NULL);
    pthread_join(print_thread, NULL);
    
    free_ppipe(p);
    free_ppipe(op);

    /* Last thing that main() should do */
    pthread_exit(NULL);
}
