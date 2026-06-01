CC      = gcc
CFLAGS  = -O2 -Wall
LDFLAGS = -lpthread

all: sharedCounter

sharedCounter: sharedCounter.c ../utils.c
	$(CC) $(CFLAGS) -o $@ sharedCounter.c ../utils.c $(LDFLAGS)

clean:
	rm -f sharedCounter
