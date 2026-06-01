CC      = gcc
CFLAGS  = -O2 -Wall -fopenmp
LDFLAGS = -lm

all: spmv

spmv: spmv.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

clean:
	rm -f spmv
