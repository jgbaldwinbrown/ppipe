all: mutex1 mutex2 mutex3 ppipe.o test test_circarr ppipe_merger.o

clean:
	-rm mutex1 mutex2 mutex3 *.o test

CC := gcc
CFLAGS := -Wall -Wextra -Wpedantic -O3
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

test.o: test.c ppipe.c ppipe.h
	$(CC) $(CFLAGS) $(LIBS) -c $<

test: test.o ppipe.o
	$(CC) $(CFLAGS) $(LIBS) $^ -o $@

test_circarr: test_circarr.o circarr.o
	$(CC) $(CFLAGS) $(LIBS) $^ -o $@
