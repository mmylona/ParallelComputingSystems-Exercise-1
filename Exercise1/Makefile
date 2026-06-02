CC      = gcc
CFLAGS  = -O2 -Wall -fopenmp -I..
LDFLAGS = -lpthread -lm

all: multiplicationOfPolynomials

multiplicationOfPolynomials: multiplicationOfPolynomials.c ../utils.c ../utils.h
	$(CC) $(CFLAGS) -o $@ multiplicationOfPolynomials.c ../utils.c $(LDFLAGS)

clean:
	rm -f multiplicationOfPolynomials
