#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include "../utils.h"
#include <time.h>

long long sharedCounter = 0;
long long iterationsPerThread = 0;
int sync_mode = 0; 

pthread_mutex_t lock;
pthread_rwlock_t rw_lock;

void* threadFunction(void* arg) {
    for (long long i = 0; i < iterationsPerThread; i++) {
        if (sync_mode == 1) { // Mutex
            pthread_mutex_lock(&lock);
            sharedCounter++;
            pthread_mutex_unlock(&lock);
        } else if (sync_mode == 2) { // RWLock
            pthread_rwlock_wrlock(&rw_lock);
            sharedCounter++;
            pthread_rwlock_unlock(&rw_lock);
        } else if (sync_mode == 3) { // Atomic
            __atomic_fetch_add(&sharedCounter, 1, __ATOMIC_SEQ_CST);
        }
    }
    return NULL;
}

int main(int argc, char* argv[]) {
    if (argc < 4) return 1;

    int nt = atoi(argv[1]);
    iterationsPerThread = atoll(argv[2]);
    if (!strcmp(argv[3], "mutex")) sync_mode = 1;
    else if (!strcmp(argv[3], "rwlock")) sync_mode = 2;
    else if (!strcmp(argv[3], "atomic")) sync_mode = 3;

    pthread_t* threads = malloc(nt * sizeof(pthread_t));
    pthread_mutex_init(&lock, NULL);
    pthread_rwlock_init(&rw_lock, NULL);

    double t_start = now_sec();
    for (int i = 0; i < nt; i++) pthread_create(&threads[i], NULL, threadFunction, NULL);
    for (int i = 0; i < nt; i++) pthread_join(threads[i], NULL);
    double t_end = now_sec() - t_start;

    int ok = (sharedCounter == (long long)nt * iterationsPerThread);
    printf("%.6f %s\n", t_end, ok ? "YES" : "NO");

    free(threads);
    pthread_mutex_destroy(&lock);
    pthread_rwlock_destroy(&rw_lock);
    return 0;
}