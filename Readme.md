# ppipe

`ppipe` is a C library that gives access to easy-to-use Unix pipe-like
structures in C for performing parallel operations in a coroutine-like manner. 
The library uses simple structs locked with mutexes to simulate pipes, and
provides generic functions for reading from and writing to these pipes.
Because the pipes are mutex-locked, multiple threads can write to and read
from the pipes atomically, and are scheduled automatically by the kernel.
`ppipe` uses `pthreads.h` for mutex locking, and Unix-style POSIX threads
are the suggested (and only tested) threading implementation to be used
with `ppipe`.

## Introduction

There are two classes of problems with concurrency in C programs. The first
set consists of limitations of the C language itself, which was originally
designed to work on the single-processor PDP-11. C had no thought given to
concurrency, though the C standard library is actually quite well suited to
coroutine-type concurrency through its interactions with Unix pipes. Until
C11 was released, there was no standard threading implementation at all,
though the POSIX pthreads implementation filled that void quite handily,
and has become a de facto standard. Because C11 threads are not yet
widely used or supported by compilers, pthreads have become commonly used
throughout the C community. Thus, the inherent threadlessness of C has
largely been solved by libraries.

The second set of problems with concurrency consists of challenges inherent to
concurrent programming. The usual suspects here are non-atomic changes to
shared memory structures, race conditions, deadlocks, and livelocks. Several
patterns for concurrency have been developed that help to prevent these issues,
two of which are used in this library.

The first of these is parallelism, in which a job
that consists of many small tasks is split up into these individual tasks,
farmed out to threads, and then the results of all individual tasks are joined
together. This approach is exceedingly simple to implement, allows for massive
parallelization of simple tasks, and can lead to dramatic speed increases,
but it is inherently inflexible. The only jobs that can be parallelized are those
that involve numerous, usually identical, tasks that do not depend on each others'
results.

A more flexible approach to concurrency is the paradigm of coroutines.
Coroutines are a fairly loosely defined concept that is often described as two
or more functions (the coroutines) that, at specified places in their code,
stop their progress and yield to their sister function, which then yields back
to the original function at an appropriate time. I have always found this
definition confusing, and I find it much easier to describe coroutines in terms
of things that programmers are already familiar with. For example, Unix
pipelines consist of a set of processes, each of which takes a result from a
pipe when one is available, then produces a partial result and feeds it into
the pipe buffer, then blocks until the buffer is emptied before proceeding to
execute more of its code. Each of these processes is able to operate in
parallel, only blocking when they either have no data on which to operate or no
space in which to write output.

What is great about coroutines is that, even though they consist of multiple
simultaneous threads of execution that depend on each others' output, they
avoid all of the problems of concurrency mentioned above: they never deadlock,
livelock, race, or use shared memory non-atomically.  This is one of the few
programming patterns that successfully makes use of complex concurrency while
avoiding these crucial problems.

C's `pthreads.h` library has no support for coroutines, though one can imagine
implementing them the same way that the Unix operating system does: make a
small shared memory space, use atomic locking to prevent multiple simultaneous
reads and writes, then hook readers and writers up to it so they can pass data
to each other. Unlike Unix pipes, which can only have one reader or writer, a
within-process pipe system can have a lightweight thread for each reader and
writer, and can have multiple readers and multiple writers per pipe without
conflict. This allows for straightforward implementation of serial concurrency
or parallelization.

## Installation

This library may come as a shared object in the future. For now, to use this library,
simply copy the source files into your build directory, build them with the included Makefile,
and include the main header as follows:

```c
#include "ppipe.h"
```

## Usage

### Making a pipe

Initialize pipes as follows, where the only argument is the size of the object that will be passed into the pipe buffer:

```c
struct ppipe p = init_ppipe(sizeof(int));
```

This generates a pipe to which you can read and write objects of size `sizeof(int)`. When you're done, free it with:

```c
free_ppipe(p);
```

### Attaching a writer to a pipe

A pipe must have at least one reader and one writer to function, or else any attached readers or writers will block
indefinitely. Here's is an example of a function that uses `ppipe_write` to write to a pipe:

```c
void *generate_nums(void *inptr) {
    int err=-1;
    struct int_generator *gen = (struct int_generator *) inptr;
    for (int i=gen->start; i<gen->end; i+=gen->step) {
        ppipe_write(gen->p, &i);
    }
    ppipe_close(gen->p);
    pthread_exit(NULL);
}
```

`generate_nums` takes a void pointer as its only argument in order to be
compatible with pthreads. All of its argument information is contained in a
struct of type `int_generator`. Here is how to attach `generate_nums` to pipe
`p`:

```c
struct int_generator gen;
gen.start = 0;
gen.end = 100;
gen.step = 2;
gen.p = &p;

int rc;
pthread_t gen_thread;

rc = pthread_create(&gen_thread, NULL, generate_nums, (void *)&gen);
if (rc){
    printf("ERROR; return code from pthread_create() is %d\n", rc);
    exit(-1);
}
```

### Attaching a reader to a pipe

Readers attach to pipes just as writers do, but they use the function `ppipe_read` to
read information from said pipes. Here is an example of a pthreads-compatible reader
function:

```c
void *print_nums(void *inptr) {
    int i = 0;
    bool closed = false;
    struct ppipe *p = (struct ppipe *) inptr;
    ppipe_read(p, &i, &closed);
    while (!closed) {
        printf("%d\n", i);
        ppipe_read(p, &i, &closed);
    }
    pthread_exit(NULL);
}
```

`print_nums` accepts a ppipe pointer as its only argument, then reads integers from the pipe and prints
them to stdout. Here is how to begin execution of `print_nums` and attach it to `struct ppipe p`:

```c
pthread_t print_thread;

rc = pthread_create(&print_thread, NULL, print_nums, (void *)&p);
if (rc){
    printf("ERROR; return code from pthread_create() is %d\n", rc);
    exit(-1);
}
```

### Running all threads

Once both of the above pipe-interacting threads have been created, we can use the following code to make sure
the threads run to completion:

```c
pthread_join(gen_thread, NULL);
pthread_join(print_thread, NULL);
```

## Example

For a full example that produces a simple integer generator-multiplier-printer with the below structure, see `test.c`:

![test](https://github.com/jgbaldwinbrown/ppipe/raw/master/test.pdf)

## Sorted joins of multiple threads

One of the big advantages of this type of concurrent programming is the ease of parallelizing slow steps. If a particular
coroutine takes in one input at a time from a ppipe, acts on it without regard for other inputs, then
outputs to another ppipe, it is trivial to parallelize that step by simply adding extra copies of that coroutine
that read from and write to the same ppipe.
There is one side effect of such parallelization, however: because the multiple instances of the coroutine
are running asynchronously, they may not write their data in the same order they read
it, with the overall result that any sorted inputs will become unsorted outputs. There are two ways
to remedy this problem. First, as long as the outputs are indexed such that a later program can
identify the correct sort order of the outputs, a later program can sort them. If, for example,
one were doing numerical calculations, the numbers of interest could be transported to and from
pipes in a struct such as:

```c
struct indexed_int {
    size_t index;
    int value;
}
```

Here, a later step in a pipeline could re-order the data points using the attached indices.
There are two downsides to this approach, though. First, sorting the data requires a complete copy of the
data to be held either in memory or on disk, making otherwise-large-file-friendly programs
into memory hogs. Second, the sorting program cannot output anything until the entire input
has been read, preventing practical concurrency.

The second solution solves both of these problems. In this solution, we pass around indexed
structs as above, but instead of sorting them, we add them to an internal buffer according to their indices,
then output items from the buffer in order, as the correct items are added to the buffer.
One can imagine the following hypothetical buffer:

```
index: 0 --- 1 --- 2 --- 3 --- 4 --- 5 --- ...
value: 0 --- 0 --- 0 --- 0 --- 0 --- 0 --- ...
full:  0 --- 0 --- 0 --- 0 --- 0 --- 0 --- ...
```

That could be filled with values, with the `full` array keeping track of whether a given indexed
space in memory is currently filled with a valid value. If we add the values 9 and 7 to indices
3 and 4, we get the following:

```
index: 0 --- 1 --- 2 --- 3 --- 4 --- 5 --- ...
value: 0 --- 0 --- 0 --- 9 --- 7 --- 0 --- ...
full:  0 --- 0 --- 0 --- 1 --- 1 --- 0 --- ...
```

The values at 3 and 4 are filled, now, but they cannot yet be output, because we must wait for earlier
values to be output in order to output in sorted order. Now, imagine that the data at index 0 gets added to the
buffer:

```
index: 0 --- 1 --- 2 --- 3 --- 4 --- 5 --- ...
value: 8 --- 0 --- 0 --- 9 --- 7 --- 0 --- ...
full:  1 --- 0 --- 0 --- 1 --- 1 --- 0 --- ...
```

At this point, we can now output the value at index 0, mark index 0 as no longer full, and begin waiting for
index 1:

```
index: 0 --- 1 --- 2 --- 3 --- 4 --- 5 --- ...
value: 8 --- 0 --- 0 --- 9 --- 7 --- 0 --- ...
full:  0 --- 0 --- 0 --- 1 --- 1 --- 0 --- ...
```

This allows us to accept data from a pipe in unsorted order, put it into an array in sorted order without having
to go through a sort operation, then print out the data in sorted order.
In the worst case scenario, where the 0th indexed value is the last one added to the buffer, we must read all input
before we can output even a single value, but because the coroutines producing the input are producing it based on initially sorted
input, we expect that the low-indexed inputs will be added to the buffer long before the high indexed inputs will be added,
and we can incrementally output the values in the buffer without ever storing all data in the buffer simultaneously.

Note that this is not unlike a pigeonhole sort, but with the condition that no two elements can ever use the same key,
and all keys from 0 to `n-1` are used, where `n` is the total number of indexed items.

The final remaining problem is, how do we go about implementing such a buffer? A typical C array used in the fashion
above would need to be resized to be larger whenever new inputs were added, but could not be resized to be smaller
when values from the buffer were removed because that would throw off the indexing. We could add values to the array,
then shift them left once a sufficient amount of space is available in the array, but that means a large amount of unnecessary
data copying. A linked list is not effective here, as the list would need to be walked every time a value was inserted
into the middle of the list. A b-tree or similar has the same problem of walking as a linked list, though it could at least
be walked in `O(log(n))` time rather than `O(n)` time. The best implementation that I could identify here was a
circular array, using modular arithmetic to treat a linear array like a circular queue. Unlike a typical queue, which
only has values added to the end and read from the start, this queue has values added anywhere along it, and read only from
the start. When the queue becomes too large to be held in the buffer, it is re-sized and all data is copied. Assuming the queue
does not continue growing in size throughout its use, this should only require `O(log(n))` copies, where `n` is the largest size
that the queue ever grows to.

### Usage

The function `ppipe_merge` is available from the header `ppipe_merger.h`, and has the
following API:

```c
struct ppipe_merger {
    struct ppipe *p;
    struct ppipe *op;
    size_t (*indexer) (void *);
};

void *ppipe_merge(void *inptr);
```

`ppipe_merge` is designed to work with `pthread`s, and needs to have a pointer to a struct of type `ppipe_merger` passed to it. `ppipe_merger` contains an input pipe, an output pipe, and a function for indexing the values that are 
to be sorted. Here's an example that uses it:

```c

/* assume that p is a ppipe that is being filled with unsorted ints */
/* assume that op is a ppipe that needs to be filled with sorted ints */

size_t index_int(void *input) {
    size_t output;
    int temp;
    memcpy(&temp, input, sizeof(int));
    output = temp;
    return(output);
}

int main() {
    
    ...
    
    struct ppipe_merger merger;
    merger.p = p;
    merger.op = op;
    merger.indexer = index_int;
    
    int rc = pthread_create(&print_thread, NULL, print_nums, (void *)&p);
    if (rc){
        printf("ERROR; return code from pthread_create() is %d\n", rc);
        exit(-1);
    }
    
    ...
    
    pthread_join(print_thread, NULL);
    
    ...
    
}

A complete example of the use of `ppipe_merger` and `ppipe_merge` is available in `test_merger.c`.
