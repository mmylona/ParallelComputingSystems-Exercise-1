CC      = gcc
CFLAGS  = -O2 -Wall -Wextra
LDFLAGS = -lpthread

TARGETS = barrier_pthread barrier_condvar barrier_sense

.PHONY: all clean

all: $(TARGETS)

barrier_pthread: barrier.c ../utils.c ../utils.h
	$(CC) $(CFLAGS) -DBARRIER_PTHREAD -o $@ barrier.c ../utils.c $(LDFLAGS)

barrier_condvar: barrier.c ../utils.c ../utils.h
	$(CC) $(CFLAGS) -DBARRIER_CONDVAR -o $@ barrier.c ../utils.c $(LDFLAGS)

barrier_sense: barrier.c ../utils.c ../utils.h
	$(CC) $(CFLAGS) -DBARRIER_SENSE -o $@ barrier.c ../utils.c $(LDFLAGS)

clean:
	rm -f $(TARGETS)