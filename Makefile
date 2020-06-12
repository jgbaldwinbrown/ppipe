all: mutex1 mutex2 mutex3 ppipe.o test test_circarr ppipe_merger.o test_merger test_merger_simple test.pdf

clean:
	-rm mutex1 mutex2 mutex3 *.o test test_circarr test_merger

CC := gcc
CFLAGS := -Wall -Wextra -Wpedantic -O3 -g
LIBS := -pthread

mutex1: mutex1.c
	$(CC) $(CFLAGS) $(LIBS) -o $@ $<

mutex2: mutex2.c
	$(CC) $(CFLAGS) $(LIBS) -o $@ $<

mutex3: mutex3.c
	$(CC) $(CFLAGS) $(LIBS) -o $@ $<

mutex4: mutex4.c
	$(CC) $(CFLAGS) $(LIBS) -o $@ $<

ppipe.o: ppipe.c ppipe.h
	$(CC) $(CFLAGS) $(LIBS) -c $<

circarr.o: circarr.c circarr.h
	$(CC) $(CFLAGS) $(LIBS) -c $<

ppipe_merger.o: ppipe_merger.c ppipe_merger.h circarr.h ppipe.h
	$(CC) $(CFLAGS) $(LIBS) -c $<

indexed_ints.o: indexed_ints.c indexed_ints.h ppipe.h
	$(CC) $(CFLAGS) $(LIBS) -c $<

test.o: test.c ppipe.c ppipe.h
	$(CC) $(CFLAGS) $(LIBS) -c $<

test: test.o ppipe.o
	$(CC) $(CFLAGS) $(LIBS) $^ -o $@

test.pdf: test.dot
	cat $^ | dot -Nshape=box -Tpdf > $@

test_circarr: test_circarr.o circarr.o indexed_ints.o ppipe.o
	$(CC) $(CFLAGS) $(LIBS) $^ -o $@

test_merger: test_merger.o ppipe_merger.o indexed_ints.o ppipe.o circarr.o
	$(CC) $(CFLAGS) $(LIBS) $^ -o $@

test_merger_simple: test_merger_simple.o ppipe_merger.o indexed_ints.o ppipe.o circarr.o
	$(CC) $(CFLAGS) $(LIBS) $^ -o $@
