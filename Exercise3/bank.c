
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <math.h>
#include <time.h>
#include <unistd.h>
#include "../utils.h"


#define SCHEME_COARSE_MUTEX 0
#define SCHEME_FINE_MUTEX   1
#define SCHEME_COARSE_RW    2
#define SCHEME_FINE_RW      3 

static int    NUM_ACCOUNTS;
static long long TRANS_PER_THREAD;
static int    QUERY_PCT;
static int    NUM_THREADS;
static int    EXTENDED_CS = 0;   // set to 1 for the extended version 

static int SCHEME;

static long long *accounts;

// locks 
static pthread_mutex_t  coarse_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_rwlock_t coarse_rw    = PTHREAD_RWLOCK_INITIALIZER;
static pthread_mutex_t  *fine_mutexes;
static pthread_rwlock_t *fine_rws;

static long long initialSum = 0;

typedef struct {
    int id;
} threadArg;

static inline void transfer(int from, int to, long long amount) {
    int first, second;

    switch (SCHEME) {
    case SCHEME_COARSE_MUTEX:
        pthread_mutex_lock(&coarse_mutex);
        accounts[from] -= amount;
        accounts[to]   += amount;
        pthread_mutex_unlock(&coarse_mutex);
        break;
    case SCHEME_FINE_MUTEX:
        // lock in order to avoid deadlock
        first = from < to ? from : to;
        second = from < to ? to : from;
        pthread_mutex_lock(&fine_mutexes[first]);
        pthread_mutex_lock(&fine_mutexes[second]);
        accounts[from] -= amount;
        accounts[to] += amount;
        pthread_mutex_unlock(&fine_mutexes[first]);
        pthread_mutex_unlock(&fine_mutexes[second]);
        break;
    case SCHEME_COARSE_RW:
        pthread_rwlock_wrlock(&coarse_rw);
        accounts[from] -= amount;
        accounts[to]   += amount;
        pthread_rwlock_unlock(&coarse_rw);
        break;
    case SCHEME_FINE_RW:
        first = from < to ? from : to;
        second = from < to ? to : from;
        pthread_rwlock_wrlock(&fine_rws[first]);
        pthread_rwlock_wrlock(&fine_rws[second]);
        accounts[from] -= amount;
        accounts[to] += amount;
        pthread_rwlock_unlock(&fine_rws[first]);
        pthread_rwlock_unlock(&fine_rws[second]);
        break;
    }
}

// query balance 
static inline long long query(int acc) {
    long long balance = 0;

    switch (SCHEME) {
    case SCHEME_COARSE_MUTEX:
        pthread_mutex_lock(&coarse_mutex);
        balance = accounts[acc];
        if (EXTENDED_CS) {
            usleep(10); 
        }
        pthread_mutex_unlock(&coarse_mutex);
        break;
    case SCHEME_FINE_MUTEX:
        pthread_mutex_lock(&fine_mutexes[acc]);
        balance = accounts[acc];
        if (EXTENDED_CS) {
            usleep(10);
        }
        pthread_mutex_unlock(&fine_mutexes[acc]);
        break;
    case SCHEME_COARSE_RW:
        pthread_rwlock_rdlock(&coarse_rw);
        balance = accounts[acc];
        if (EXTENDED_CS) {
            usleep(10);
        }
        pthread_rwlock_unlock(&coarse_rw);
        break;
    case SCHEME_FINE_RW:
        pthread_rwlock_rdlock(&fine_rws[acc]);
        balance = accounts[acc];
        if (EXTENDED_CS) {
            usleep(10);
        }
        pthread_rwlock_unlock(&fine_rws[acc]);
        break;
    default:
        balance = 0;
    }
    return balance;
}

static void *worker(void *arg) {
    int tid = ((threadArg *)arg)->id;

    unsigned int seed = (unsigned int)(42 + tid);
    long long local_query_sum = 0;

    for (long long t = 0; t < TRANS_PER_THREAD; t++) {
        int is_query = ((rand_r(&seed) % 100) < QUERY_PCT);
        if (is_query) {
            int acc = rand_r(&seed) % NUM_ACCOUNTS;
            local_query_sum += query(acc);
        } else {
            int from   = rand_r(&seed) % NUM_ACCOUNTS;
            int to     = rand_r(&seed) % NUM_ACCOUNTS;
            while (to == from) to = rand_r(&seed) % NUM_ACCOUNTS;
            long long amount = (rand_r(&seed) % 100) + 1;
            transfer(from, to, amount);
        }
    }
    // suppress unused warning
    (void)local_query_sum;
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc < 6 || argc > 7) {
        fprintf(stderr,
            "Usage: %s <num_accounts> <trans_per_thread> <query_pct> "
            "<coarse_mutex|fine_mutex|coarse_rw|fine_rw> <num_threads> [extended_cs=0|1]\n",
            argv[0]);
        return 1;
    }

    NUM_ACCOUNTS      = atoi(argv[1]);
    TRANS_PER_THREAD  = atoll(argv[2]);
    QUERY_PCT         = atoi(argv[3]);
    NUM_THREADS       = atoi(argv[5]);
    if (argc == 7) EXTENDED_CS = atoi(argv[6]);

    if      (strcmp(argv[4], "coarse_mutex") == 0) SCHEME = SCHEME_COARSE_MUTEX;
    else if (strcmp(argv[4], "fine_mutex")   == 0) SCHEME = SCHEME_FINE_MUTEX;
    else if (strcmp(argv[4], "coarse_rw")    == 0) SCHEME = SCHEME_COARSE_RW;
    else if (strcmp(argv[4], "fine_rw")      == 0) SCHEME = SCHEME_FINE_RW;
    else { fprintf(stderr, "Unknown scheme: %s\n", argv[4]); return 1; }

    // allocate and init accounts with random balances
    accounts = malloc(NUM_ACCOUNTS * sizeof(long long));
    srand(123);
    for (int i = 0; i < NUM_ACCOUNTS; i++) {
        accounts[i] = (rand() % 10000) + 1;
        initialSum += accounts[i];
    }

    // allocate per-account locks
    fine_mutexes = malloc(NUM_ACCOUNTS * sizeof(pthread_mutex_t));
    fine_rws     = malloc(NUM_ACCOUNTS * sizeof(pthread_rwlock_t));
    for (int i = 0; i < NUM_ACCOUNTS; i++) {
        pthread_mutex_init(&fine_mutexes[i], NULL);
        pthread_rwlock_init(&fine_rws[i], NULL);
    }

    printf("Accounts        : %d\n", NUM_ACCOUNTS);
    printf("Trans/thread    : %lld\n", TRANS_PER_THREAD);
    printf("Query %%         : %d\n", QUERY_PCT);
    printf("Lock scheme     : %s\n", argv[4]);
    printf("Threads         : %d\n", NUM_THREADS);
    printf("Extended CS     : %s\n", EXTENDED_CS ? "yes" : "no");

    pthread_t *threads = malloc(NUM_THREADS * sizeof(pthread_t));
    threadArg      *args    = malloc(NUM_THREADS * sizeof(threadArg));

    double t0 = now_sec();
    for (int t = 0; t < NUM_THREADS; t++) {
        args[t].id = t;
        pthread_create(&threads[t], NULL, worker, &args[t]);
    }
    for (int t = 0; t < NUM_THREADS; t++)
        pthread_join(threads[t], NULL);
    double elapsed = now_sec() - t0;

    // verification
    long long final_sum = 0;
    for (int i = 0; i < NUM_ACCOUNTS; i++) final_sum += accounts[i];

    printf("Parallel time   : %.6f s\n", elapsed);
    printf("Initial sum     : %lld\n", initialSum);
    printf("Final sum       : %lld\n", final_sum);
    printf("Verification    : %s\n", (initialSum == final_sum) ? "OK" : "FAIL");

    // cleanup
    for (int i = 0; i < NUM_ACCOUNTS; i++) {
        pthread_mutex_destroy(&fine_mutexes[i]);
        pthread_rwlock_destroy(&fine_rws[i]);
    }
    pthread_mutex_destroy(&coarse_mutex);
    pthread_rwlock_destroy(&coarse_rw);
    free(fine_mutexes); free(fine_rws);
    free(accounts); free(threads); free(args);
    return 0;
}
