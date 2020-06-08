#include "ppipe.h"

struct indexed_int {
    size_t index;
    int value;
};

size_t index_indexed_int(void *inptr);
void *generate_indexed_nums(void *inptr);
void *multiply_indexed_nums(void *inptr);
void *print_indexed_nums(void *inptr);
void indexed_int_print(struct indexed_int);
