CC      = gcc
CFLAGS  = -O2 -Wall -fopenmp

all: mergesort

mergesort: mergeSort.c
	$(CC) $(CFLAGS) -o $@ mergeSort.c

clean:
	rm -f mergesort