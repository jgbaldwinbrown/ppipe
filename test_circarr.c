#include <stdio.h>
#include "circarr.h"

size_t int_indexer(void *input) {
    int i = 0;
    size_t j = 0;
    memcpy(&i, input, sizeof(int));
    j=i;
    return(j);
}

int main() {
    struct circarr c = init_circarr(5, sizeof(int), int_indexer);
    for (i=0; i<10; i++) {
        circarr_add(c, &i);
    }
    for (j=0; j<10; j++) {
        if (circarr_full(c, j)) {
            printf("%d\n", circarr_pop(c, j));
        }
    }
    return(0);
}
