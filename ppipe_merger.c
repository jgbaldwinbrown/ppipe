#include "ppipe_merger.h"
#include "circarr.h"
#include <stdlib.h>

void *ppipe_merge(void *inptr) {
    struct ppipe_merger *merger = inptr;
    struct circarr c = init_circarr(20, merger->p->member_size, merger->indexer);
    bool closed = false;
    printf("merger->p->member_size: %ld\n", merger->p->member_size);
    char *inbuf = calloc(1, merger->p->member_size);
    char *outbuf = calloc(1, merger->p->member_size);
    
    while (!closed) {
        ppipe_read(merger->p, inbuf, &closed);
        if (closed) {
            break;
        }
        circarr_add(&c, inbuf);
        while (circarr_poppable(c)) {
            circarr_pop(&c, outbuf);
            ppipe_write(merger->op, outbuf, false);
        }
    }
    while (circarr_poppable(c)) {
        circarr_pop(&c, outbuf);
        ppipe_write(merger->op, outbuf, false);
    }
    ppipe_write(merger->op, outbuf, true);
    printf("c.buf: %p; c.full: %p; inbuf: %p; outbuf: %p", c.buf, c.full, inbuf, outbuf);
    circarr_free(c);
    free(inbuf);
    free(outbuf);
    pthread_exit(NULL);
}
