#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/time.h>
#include "utils.h"
#include <omp.h>


typedef struct {
    long long *coef;
    int degree;
} Polynomial;

typedef struct {
    const Polynomial *A, *B;
    Polynomial       *C;
    int         kStart, kEnd;  
} ThreadArgs;

// Generate a random degree-n polynomial χ
static Polynomial createRandomPolynomial(int n, unsigned int *seed) {
    Polynomial p;
    p.degree = n;
    p.coef   = (long long *)malloc((n + 1) * sizeof(long long));
    if (!p.coef) { 
        perror("malloc"); exit(EXIT_FAILURE); 
    }

    for (int i = 0; i <= n; i++) {
        long long v = rand_r(seed) % 100 + 1;
        if (rand_r(seed) % 2 == 0)
            v = -v;
        p.coef[i] = v;
    }
    return p;
}

// Allocate a zero polynomial of degree a+b
static Polynomial allocateZeroResultPolynomial(int degreeA, int degreeB) {
    Polynomial r;
    r.degree = degreeA + degreeB;
    r.coef   = (long long *)calloc(degreeA + degreeB + 1, sizeof(long long));
    if (!r.coef) { 
        perror("calloc"); exit(EXIT_FAILURE); 
    }
    return r;
}

static void serialMultiply(const Polynomial *A, const Polynomial *B, Polynomial *C) {
    int degreeA = A->degree;
    int degreeB = B->degree;
    for (int i = 0; i <= degreeA; i++)
        for (int j = 0; j <= degreeB; j++)
            C->coef[i + j] += (long long)A->coef[i] * B->coef[j];
}

static void *threadWorker(void *arg) {
    ThreadArgs *a = (ThreadArgs *)arg;
    int degreeA = a->A->degree;
    int degreeB = a->B->degree;

    for (int k = a->kStart; k <= a->kEnd; k++) {
        long long sum = 0;
        int start = (k - degreeB > 0) ? k - degreeB : 0;
        int end = (k < degreeA) ? k : degreeA;
        for (int i = start; i <= end; i++)
            sum += a->A->coef[i] * a->B->coef[k - i];
        a->C->coef[k] = sum;
    }
    return NULL;
                
}

static void usePthreads(const Polynomial *A, const Polynomial *B, Polynomial *C, int num_threads) {
    // number of output coefficients 
    int total = A->degree + B->degree + 1;  
    pthread_t   *threadIds = malloc(num_threads * sizeof(pthread_t));
    ThreadArgs  *threadData = malloc(num_threads * sizeof(ThreadArgs));
    if (!threadIds || !threadData) { perror("malloc"); exit(EXIT_FAILURE); }

    int workPerThread = total / num_threads;
    int remainder  = total % num_threads;
    int start = 0;

    for (int t = 0; t < num_threads; t++) {
        int end = start + workPerThread - 1 + (t < remainder ? 1 : 0);
        threadData[t].A       = A;
        threadData[t].B       = B;
        threadData[t].C       = C;
        threadData[t].kStart = start;
        threadData[t].kEnd   = end;
        pthread_create(&threadIds[t], NULL, threadWorker, &threadData[t]);
        start = end + 1;
    }
    for (int t = 0; t < num_threads; t++)
        pthread_join(threadIds[t], NULL);

    free(threadIds);
    free(threadData);
}

static void useOpenmp(const Polynomial *A, const Polynomial *B, Polynomial *C, int num_threads) {
    int degreeA = A->degree;
    int degreeB = B->degree;
    int total = degreeA + degreeB;

    omp_set_num_threads(num_threads);

#pragma omp parallel for schedule(dynamic, 64)
    for (int k = 0; k <= total; k++) {
        long long sum = 0;
        int start = (k - degreeB > 0) ? k - degreeB : 0;
        int end = (k < degreeA) ? k : degreeA;
        for (int i = start; i <= end; i++)
            sum += (long long)A->coef[i] * B->coef[k - i];
        C->coef[k] = sum;
    }
}

// Verify two result polynomials are identical
static int isEqual(const Polynomial *P, const Polynomial *Q) {
    if (P->degree != Q->degree) return 0;
    for (int i = 0; i <= P->degree; i++)
        if (P->coef[i] != Q->coef[i]) return 0;
    return 1;
}

static void freePolynomial(Polynomial *p) {
    free(p->coef);
    p->coef   = NULL;
    p->degree = -1;
}

static void usage(const char *prog) {
    fprintf(stderr, "Error: Invalid Arguments\n");
    fprintf(stderr, "Usage: %s -a <degA> -b <degB> -t <threads> -m <pthreads|openmp>\n", prog);
    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]) {
    int degreeA = -1;
    int degreeB = -1;
    int  numOfThreads = -1;
    // 0 = pthreads, 1 = openmp
    int  implementation  = -1;    

    // Check if minimum number of arguments is provided
    if (argc < 9) usage(argv[0]);

    // argument parsing 
    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "-a") && i + 1 < argc)
            degreeA = atoi(argv[++i]);
        else if (!strcmp(argv[i], "-b") && i + 1 < argc)
            degreeB = atoi(argv[++i]);
        else if (!strcmp(argv[i], "-t") && i + 1 < argc)
            numOfThreads = atoi(argv[++i]);
        else if (!strcmp(argv[i], "-m") && i + 1 < argc) {
            ++i;
            if (!strcmp(argv[i], "openmp"))   implementation = 1;
            else if (!strcmp(argv[i], "pthreads")) implementation = 0;
            else {
                fprintf(stderr, "Error: Invalid mode '%s'\n", argv[i]);
                return EXIT_FAILURE;
            }
        } else {
            fprintf(stderr, "Error: Unknown flag '%s'\n", argv[i]);
            usage(argv[0]);
        }
    }

    if (degreeA < 0 || degreeB < 0 || numOfThreads < 1 || implementation < 0) {
        fprintf(stderr, "Error: Missing or invalid parameters.\n");
        return EXIT_FAILURE;
    }

    // initialisation
    unsigned int seed = (unsigned int)time(NULL);
    Polynomial A = createRandomPolynomial(degreeA, &seed);
    Polynomial B = createRandomPolynomial(degreeB, &seed);
    Polynomial C_serial   = allocateZeroResultPolynomial(degreeA, degreeB);
    Polynomial C_parallel = allocateZeroResultPolynomial(degreeA, degreeB);

    // serial multiplication
    double t1 = now_sec();
    serialMultiply(&A, &B, &C_serial);
    double serialTime = now_sec() - t1;

    // parallel multiplication
    double t2 = now_sec();
    if (implementation == 1)
        useOpenmp(&A, &B, &C_parallel, numOfThreads);
    else
        usePthreads(&A, &B, &C_parallel, numOfThreads);
    double parallelTime = now_sec() - t2;

    // verification
    int ok = isEqual(&C_serial, &C_parallel);

    // Summary
    double speedup = (parallelTime > 0) ? (serialTime / parallelTime) : 0;
    printf("%.6f %.6f %.2f %s\n", serialTime, parallelTime, speedup, ok ? "YES" : "NO");

    freePolynomial(&A);
    freePolynomial(&B);
    freePolynomial(&C_serial);
    freePolynomial(&C_parallel);

    return ok ? EXIT_SUCCESS : EXIT_FAILURE;
}