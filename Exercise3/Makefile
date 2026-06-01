CC      = gcc
CFLAGS  = -O2 -Wall
LDFLAGS = -lpthread -lm

all: bank

bank: bank.c ../utils.c ../utils.h
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	rm -f bank
