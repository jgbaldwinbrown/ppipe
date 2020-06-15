#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "ppipe.h"

#define PIPEBUFSIZ	20

int main(int argc, char *argv[]) {
    pthread_t gen_thread;
    pthread_t tee_thread;
    pthread_t print_thread;
    pthread_t print_thread2;
    int rc;
    
    struct ppipe p = init_ppipe(sizeof(int), 1);
    struct ppipe op1 = init_ppipe(sizeof(int), 1);
    struct ppipe op2 = init_ppipe(sizeof(int), 1);
    
    struct int_generator gen;
    gen.start = 0;
    gen.end = 100;
    gen.step = 2;
    gen.p = &p;
    
    struct teer ateer;
    ateer.p = &p;
    ateer.op1 = &op1;
    ateer.op2 = &op2;

    rc = pthread_create(&gen_thread, NULL, generate_nums, (void *)&gen);
    if (rc){
        printf("ERROR; return code from pthread_create() is %d\n", rc);
        exit(-1);
    }
    
    rc = pthread_create(&tee_thread, NULL, tee, (void *)&ateer);
    if (rc){
        printf("ERROR; return code from pthread_create() is %d\n", rc);
        exit(-1);
    }
    
    rc = pthread_create(&print_thread, NULL, print_nums, (void *)&op1);
    if (rc){
        printf("ERROR; return code from pthread_create() is %d\n", rc);
        exit(-1);
    }

    rc = pthread_create(&print_thread2, NULL, print_nums, (void *)&op2);
    if (rc){
        printf("ERROR; return code from pthread_create() is %d\n", rc);
        exit(-1);
    }
    printf("made all threads\n");
 
    pthread_join(gen_thread, NULL);
    pthread_join(tee_thread, NULL);
    pthread_join(print_thread, NULL);
    pthread_join(print_thread2, NULL);
    
    free_ppipe(p);
    free_ppipe(op1);
    free_ppipe(op2);

    /* Last thing that main() should do */
    pthread_exit(NULL);
}
