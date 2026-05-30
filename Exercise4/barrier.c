#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "../utils.h"

static int       NUM_THREADS;
static long long N_ITER;

typedef struct {
    volatile int count;         // threads that have arrived 
    int          total;         // total expected threads           
    volatile int global_sense;  // current phase value (0 or 1)     
} SenseBarrier;

typedef struct {
    int             count;   // threads that have arrived      
    int             total;   // total threads                  
    int             cycle;   // alternates 0/1 between phases  
    pthread_mutex_t mutex;
    pthread_cond_t  cond;
} CondBarrier;

#ifdef BARRIER_PTHREAD

static pthread_barrier_t pbar;

static void *worker(void *arg) {
    (void)arg;
    for (long long i = 0; i < N_ITER; i++)
        pthread_barrier_wait(&pbar);
    return NULL;
}

static void run(void) {
    pthread_barrier_init(&pbar, NULL, NUM_THREADS);
    pthread_t *thr = malloc(NUM_THREADS * sizeof(pthread_t));
    if (!thr) { perror("malloc"); exit(1); }

    double t0 = now_sec();
    for (int t = 0; t < NUM_THREADS; t++)
        pthread_create(&thr[t], NULL, worker, NULL);
    for (int t = 0; t < NUM_THREADS; t++)
        pthread_join(thr[t], NULL);
    printf("pthread_barrier : %.6f s\n", now_sec() - t0);

    pthread_barrier_destroy(&pbar);
    free(thr);
}
#endif  /* BARRIER_PTHREAD */


#ifdef BARRIER_CONDVAR

static void cbarInit(CondBarrier *b, int n) {
    b->count = 0;
    b->total = n;
    b->cycle = 0;
    pthread_mutex_init(&b->mutex, NULL);
    pthread_cond_init(&b->cond, NULL);
}

static void cbarWait(CondBarrier *b) {
    pthread_mutex_lock(&b->mutex);
    int my_cycle = b->cycle;
    b->count++;
    if (b->count == b->total) {
        // Last thread to arrive: reset counter, flip cycle, wake all 
        b->count = 0;
        b->cycle = 1 - b->cycle;
        pthread_cond_broadcast(&b->cond);
    } else {
        // Wait until the cycle changes
        while (b->cycle == my_cycle)
            pthread_cond_wait(&b->cond, &b->mutex);
    }
    pthread_mutex_unlock(&b->mutex);
}

static void cbar_destroy(CondBarrier *b) {
    pthread_mutex_destroy(&b->mutex);
    pthread_cond_destroy(&b->cond);
}

static CondBarrier cbar;

static void *worker(void *arg) {
    (void)arg;
    for (long long i = 0; i < N_ITER; i++)
        cbarWait(&cbar);
    return NULL;
}

static void run(void) {
    cbarInit(&cbar, NUM_THREADS);
    pthread_t *thr = malloc(NUM_THREADS * sizeof(pthread_t));
    if (!thr) { perror("malloc"); exit(1); }

    double t0 = now_sec();
    for (int t = 0; t < NUM_THREADS; t++)
        pthread_create(&thr[t], NULL, worker, NULL);
    for (int t = 0; t < NUM_THREADS; t++)
        pthread_join(thr[t], NULL);
    printf("condvar barrier : %.6f s\n", now_sec() - t0);

    cbar_destroy(&cbar);
    free(thr);
}
#endif  /* BARRIER_CONDVAR */

#ifdef BARRIER_SENSE

static SenseBarrier sbar;

// Each thread flips it before incrementing the shared counter
static __thread int local_sense = 0;

static void sbar_wait(SenseBarrier *b) {
    // Flip local sense to mark entry into the new phase 
    local_sense = 1 - local_sense;
    int ls = local_sense;

    // Atomically announce arrival 
    int prev = __atomic_fetch_add(&b->count, 1, __ATOMIC_ACQ_REL);

    if (prev + 1 == b->total) {
        __atomic_store_n(&b->count, 0, __ATOMIC_RELEASE);
        // Publish new phase 
        __atomic_store_n(&b->global_sense, ls, __ATOMIC_RELEASE);
    } else {
        // Spin until the last thread publishes the new phase
        while (__atomic_load_n(&b->global_sense, __ATOMIC_ACQUIRE) != ls)
            ;  // busy-wait
    }
}

static void *worker(void *arg) {
    (void)arg;
    for (long long i = 0; i < N_ITER; i++)
        sbar_wait(&sbar);
    return NULL;
}

static void run(void) {
    sbar.count        = 0;
    sbar.total        = NUM_THREADS;
    sbar.global_sense = 0;

    pthread_t *thr = malloc(NUM_THREADS * sizeof(pthread_t));
    if (!thr) { perror("malloc"); exit(1); }

    double t0 = now_sec();
    for (int t = 0; t < NUM_THREADS; t++)
        pthread_create(&thr[t], NULL, worker, NULL);
    for (int t = 0; t < NUM_THREADS; t++)
        pthread_join(thr[t], NULL);
    printf("sense barrier   : %.6f s\n", now_sec() - t0);

    free(thr);
}
#endif  /* BARRIER_SENSE */

int main(int argc, char *argv[])
{
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <num_threads> <iterations>\n", argv[0]);
        return 1;
    }
    NUM_THREADS = atoi(argv[1]);
    N_ITER      = atoll(argv[2]);

    if (NUM_THREADS < 1 || N_ITER < 1) {
        fprintf(stderr, "Error: num_threads and iterations must be >= 1\n");
        return 1;
    }

    printf("Threads         : %d\n", NUM_THREADS);
    printf("Iterations      : %lld\n", N_ITER);
    run();
    return 0;
}