#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "ppipe.h"

#define PIPEBUFSIZ	20

int main(int argc, char *argv[]) {
    pthread_t gen_thread;
    pthread_t mult_thread;
    pthread_t mult_thread2;
    pthread_t print_thread;
    pthread_t print_thread2;
    int rc;
    
    struct ppipe p = init_ppipe(sizeof(int));
    struct ppipe op = init_ppipe(sizeof(int));
    
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
    
    free_ppipe(p);
    free_ppipe(op);

    /* Last thing that main() should do */
    pthread_exit(NULL);
}
