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
        ppipe_write(gen->p, &i, false);
    }
    ppipe_write(gen->p, &err, true);
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
    while (!closed) {
        ppipe_read(p, &i, &closed);
        if (!closed) {
            printf("%d\n", i);
        }
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

![test]
(test.pdf)
