#include "ppipe.h"
#include "circarr.h"

struct ppipe_merger {
    struct ppipe *p;
    struct ppipe *op;
    size_t (*indexer) (void *);
};

void *ppipe_merge(void *inptr);
