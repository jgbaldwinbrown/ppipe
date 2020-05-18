all: mutex1 mutex2 mutex3 mutex4

clean:
	-rm mutex1 mutex2

mutex1: mutex1.c
	gcc -pthread -Wall -O3 -o $@ $^

mutex2: mutex2.c
	gcc -pthread -Wall -O3 -o $@ $^

mutex3: mutex3.c
	gcc -pthread -Wall -O3 -o $@ $^

mutex4: mutex4.c
	gcc -pthread -Wall -O3 -o $@ $^
