#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define CIRCARRSIZ 20

struct indexed_int {
    size_t index;
    int value;
};

struct indexed_int_holder {
    struct indexed_int indexed_int;
    bool filled;
};

struct thread_merger {
    struct ppipe *p;
    struct ppipe *op;
    size_t (*indexer) (struct indexed_int);
};

size_t int_indexer(struct indexed_int i) {
    return(i.index);
}

struct indexed_int_holder *double_circular_array(struct indexed_int_holder *buf, size_t *bufsiz, size_t bufpos) {
    static struct indexed_int_holder empty_holder;
    empty_holder.indexed_int.index = 0;
    empty_holder.indexed_int.value = 0;
    empty_holder.filled = false;
    
    if ((buf = realloc(buf, sizeof(struct indexed_int_holder) * (*bufsiz) * 2)) == NULL) {
        fputs("Out of memory.", stderr);
        exit(1);
    }
    for (size_t loop_i=*bufsiz; loop_i<*bufsiz; loop_i++) {
        buf[loop_i] = empty_holder;
    }
    for (size_t i=0; i<bufpos; i++) {
        buf[i+(*bufsiz)] = buf[i];
        buf[i] = empty_holder;
    }
    *bufsiz*=2;
    return(buf);
}

void *merge_threads(void *inptr) {
    struct thread_merger *merger = (struct thread_merger *) inptr;
    bool closed = false;
    struct indexed_int i;
    size_t curindex = 0;
    size_t pos_to_edit = 0;
    size_t bufsiz = PIPEBUFSIZ;
    struct indexed_int_holder *buf;
    if ((buf = malloc(sizeof(struct indexed_int_holder) * bufsiz)) == NULL) {
        fputs("Out of memory.", stderr);
        exit(1);
    }
    for (size_t loop_i=0; loop_i<bufsiz; loop_i++) {
        buf[loop_i].indexed_int.index = 0;
        buf[loop_i].indexed_int.value = 0;
        buf[loop_i].filled = false;
    }
    size_t curbufpos = 0;
    
    while(!closed) {
        i = read_int_from_ppipe(merger->p, &closed);
        if merger->indexer(i) <= curindex {
            write_int_to_ppipe(merger->p, i.value, closed);
            buf[curbufpos].filled = false;
            curbufpos = (curbufpos+1) % bufsiz;
            curindex++;
        } else {
            while (merger->indexer(i) - curindex > bufsiz) {
                buf = double_circular_array(buf, &bufsiz, curbufpos);
            }
            pos_to_edit = (curbufpos + (merger->indexer(i) - curindex))
            buf[pos_to_edit].indexed_int = i;
            buf[pos_to_edit].full = true;
        }
        while (merger->indexer(buf[curpos].indexed_int) == curindex + 1) {
            if (closed && !buf[curbufpos].full) {
                write_int_to_ppipe(merger->p, buf[curpos].indexed_int.value, closed);
            } else {
                write_int_to_ppipe(merger->p, buf[curpos].indexed_int.value, false);
            }
            curindex++;
            curbufpos = (curbufpos+1) & bufsiz;
        }
    }
    
    while (merger->indexer(buf[curpos].indexed_int) == curindex + 1) {
        if (closed && !buf[curbufpos].full) {
            write_int_to_ppipe(merger->p, buf[curpos].indexed_int.value, closed);
        } else {
            write_int_to_ppipe(merger->p, buf[curpos].indexed_int.value, false);
        }
        curindex++;
        curbufpos = (curbufpos+1) & bufsiz;
    }
    
    free(buf);
    pthread_exit(NULL);
}

