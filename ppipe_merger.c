#include "ppipe_merger.h"

void *ppipe_merge(void *inptr) {
    struct ppipe_merger *merger = inptr;
    struct circarr c = init_circarr(20, merger->p->member_size, merger->indexer);
    bool closed = false;
    int i=0;
    int j=0;
    
    while (!closed) {
        ppipe_read(merger->p, &i, &closed);
        if (closed) {
            break;
        }
        circarr_add(&c, &i);
        while (circarr_poppable(c)) {
            circarr_pop(&c, &j);
            ppipe_write(merger->op, &j, false);
        }
    }
    while (circarr_poppable(c)) {
        circarr_pop(&c, &j);
        ppipe_write(merger->op, &j, false);
    }
    ppipe_write(merger->op, &j, true);
    circarr_free(c);
    return(inptr);
}
