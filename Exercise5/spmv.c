#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

typedef struct {
    int    N;
    long  *data;   
} DenseMatrix;

// CSR representation 
typedef struct {
    int   N;
    int   nonZero;    
    long  *nonZeroValues;   
    int   *nonZeroColumnIdx;   
    int   *rowPtr;    // N+1 row pointers                
} CSRMatrix;

static DenseMatrix *allocateDense(int N) {
    DenseMatrix *m = malloc(sizeof(DenseMatrix));
    m->N    = N;
    m->data = calloc((size_t)N * N, sizeof(long));
    return m;
}

static void freeDense(DenseMatrix *m) {
    free(m->data); free(m);
}

static void freeCSR(CSRMatrix *m) {
    free(m->nonZeroValues); free(m->nonZeroColumnIdx); free(m->rowPtr); free(m);
}


static DenseMatrix *buildDense(int N, int sparsityPercent) {
    DenseMatrix *m = allocateDense(N);
    srand(42);
    for (int i = 0; i < N; i++)
        for (int j = 0; j < N; j++)
            if ((rand() % 100) >= sparsityPercent)
                m->data[(size_t)i * N + j] = (rand() % 100) + 1;
    return m;
}

static long *buildVector(int N) {
    long *v = malloc(N * sizeof(long));
    srand(99);
    for (int i = 0; i < N; i++)
        v[i] = (rand() % 10) + 1;
    return v;
}

static CSRMatrix *serialCsrBuild(DenseMatrix *d) {
    int N = d->N;
    CSRMatrix *csr = malloc(sizeof(CSRMatrix));
    csr->N      = N;
    csr->rowPtr = malloc((N + 1) * sizeof(int));
 
    csr->rowPtr[0] = 0;
    for (int i = 0; i < N; i++) {
        int cnt = 0;
        for (int j = 0; j < N; j++)
            if (d->data[(size_t)i * N + j] != 0) cnt++;
        csr->rowPtr[i + 1] = csr->rowPtr[i] + cnt;
    }
    csr->nonZero          = csr->rowPtr[N];
    csr->nonZeroValues    = malloc(csr->nonZero * sizeof(long));
    csr->nonZeroColumnIdx = malloc(csr->nonZero * sizeof(int));
 
    for (int i = 0; i < N; i++) {
        int pos = csr->rowPtr[i];
        for (int j = 0; j < N; j++) {
            long v = d->data[(size_t)i * N + j];
            if (v != 0) {
                csr->nonZeroValues[pos]    = v;
                csr->nonZeroColumnIdx[pos] = j;
                pos++;
            }
        }
    }
    return csr;
}

static CSRMatrix *parallelCsrBuild(DenseMatrix *d, int numOfThreads) {
    int N = d->N;
    CSRMatrix *csr = malloc(sizeof(CSRMatrix));
    csr->N = N;
    csr->rowPtr = malloc((N + 1) * sizeof(int));

    // count non zero values per row 
    #pragma omp parallel for num_threads(numOfThreads) schedule(static)
    for (int i = 0; i < N; i++) {
        int cnt = 0;
        for (int j = 0; j < N; j++)
            if (d->data[(size_t)i * N + j] != 0) cnt++;
        csr->rowPtr[i + 1] = cnt;  
    }
    csr->rowPtr[0] = 0;

    // prefix sum for rowPtr
    csr->rowPtr[0] = 0;
    for (int i = 0; i < N; i++)
        csr->rowPtr[i + 1] += csr->rowPtr[i];
 
    csr->nonZero = csr->rowPtr[N];
    csr->nonZeroValues = malloc(csr->nonZero * sizeof(long));
    csr->nonZeroColumnIdx = malloc(csr->nonZero * sizeof(int));

    // fill values & columnIdx in parallel
    #pragma omp parallel for num_threads(numOfThreads) schedule(static)
    for (int i = 0; i < N; i++) {
        int pos = csr->rowPtr[i];
        for (int j = 0; j < N; j++) {
            long v = d->data[(size_t)i * N + j];
            if (v != 0) {
                csr->nonZeroValues[pos]    = v;
                csr->nonZeroColumnIdx[pos] = j;
                pos++;
            }
        }
    }
    return csr;
}

static void serialSpmvCsr(CSRMatrix *A, long *x, long *y) {
    for (int i = 0; i < A->N; i++) {
        long sum = 0;
        for (int k = A->rowPtr[i]; k < A->rowPtr[i + 1]; k++)
            sum += A->nonZeroValues[k] * x[A->nonZeroColumnIdx[k]];
        y[i] = sum;
    }
}

static void parallelSpmvCsr(CSRMatrix *parallelCSR, long *currentVector, long *y, int numOfThreads) {
   #pragma omp parallel for num_threads(numOfThreads) schedule(static)
    for (int i = 0; i < parallelCSR->N; i++) {
        long sum = 0;
        for (int k = parallelCSR->rowPtr[i]; k < parallelCSR->rowPtr[i + 1]; k++)
            sum += parallelCSR->nonZeroValues[k] * currentVector[parallelCSR->nonZeroColumnIdx[k]];
        y[i] = sum;
    }
}

static void parallelSpmvDense(DenseMatrix *A, long *currentVector, long *y, int numOfThreads) {
    int N = A->N;
    #pragma omp parallel for num_threads(numOfThreads) schedule(static)
    for (int i = 0; i < N; i++) {
        long sum = 0;
        for (int j = 0; j < N; j++)
            sum += A->data[(size_t)i * N + j] * currentVector[j];
        y[i] = sum;
    }
}

static int verify(long *a, long *b, int N) {
    for (int i = 0; i < N; i++)
        if (a[i] != b[i]) return 0;
    return 1;
}


int main(int argc, char *argv[]) {
    if (argc != 5) {
        fprintf(stderr, "Usage: %s <N> <sparsity_pct> <iterations> <num_threads>\n", argv[0]);
        return 1;
    }
    int N = atoi(argv[1]);
    int sparsity = atoi(argv[2]);
    int iterations = atoi(argv[3]);
    int numOfThreads = atoi(argv[4]);

    printf("N=%d sparsity=%d iterations=%d threads=%d\n",
           N, sparsity, iterations, numOfThreads);
 
    DenseMatrix *D = buildDense(N, sparsity);
    long *inputVector = buildVector(N);
    long *yCsrSerial = malloc(N * sizeof(long));
    long *yCsrParallel = malloc(N * sizeof(long));
    long *yDense = malloc(N * sizeof(long));
    long *currentVector = malloc(N * sizeof(long));

    // CSR construction serial
    double t0 = omp_get_wtime();
    CSRMatrix *csrSerial = serialCsrBuild(D);
    double t_build_s = omp_get_wtime() - t0;
    printf("CSR_BUILD_SERIAL %.6f nnz=%d\n", t_build_s, csrSerial->nonZero);

    // CSR construction parallel
    t0 = omp_get_wtime();
    CSRMatrix *csrPar = parallelCsrBuild(D, numOfThreads);
    double t_build_p = omp_get_wtime() - t0;
    printf("CSR_BUILD_PARALLEL %.6f\n", t_build_p);

    // SpMV CSR serial
    memcpy(currentVector, inputVector, N * sizeof(long));
    t0 = omp_get_wtime();
    for (int it = 0; it < iterations; it++) {
        serialSpmvCsr(csrSerial, currentVector, yCsrSerial);
        memcpy(currentVector, yCsrSerial, N * sizeof(long));
    }
    double t_spmv_s = omp_get_wtime() - t0;
    printf("SPMV_CSR_SERIAL %.6f\n", t_spmv_s);

    // SpMV CSR parallel
    memcpy(currentVector, inputVector, N * sizeof(long));
    t0 = omp_get_wtime();
    for (int it = 0; it < iterations; it++) {
        parallelSpmvCsr(csrPar, currentVector, yCsrParallel, numOfThreads);
        memcpy(currentVector, yCsrParallel, N * sizeof(long));
    }
    double t_spmv_p = omp_get_wtime() - t0;
    printf("SPMV_CSR_PARALLEL %.6f\n", t_spmv_p);

    // SpMV Dense parallel 
    memcpy(currentVector, inputVector, N * sizeof(long));
    t0 = omp_get_wtime();
    for (int it = 0; it < iterations; it++) {
        parallelSpmvDense(D, currentVector, yDense, numOfThreads);
        memcpy(currentVector, yDense, N * sizeof(long));
    }
    double t_dense_p = omp_get_wtime() - t0;
    printf("SPMV_DENSE_PARALLEL %.6f\n", t_dense_p);

    // Verification: CSR parallel == CSR serial 
    long *refVec = malloc(N * sizeof(long));
    long *parVec = malloc(N * sizeof(long));
    memcpy(refVec, inputVector, N * sizeof(long));
    memcpy(parVec, inputVector, N * sizeof(long));
    long *tmpRef = malloc(N * sizeof(long));
    long *tmpPar = malloc(N * sizeof(long));
    int ok = 1;
    for (int it = 0; it < iterations; it++) {
        serialSpmvCsr(csrSerial, refVec, tmpRef);
        parallelSpmvCsr(csrPar,  parVec, tmpPar, numOfThreads);
        if (!verify(tmpRef, tmpPar, N)) { ok = 0; break; }
        memcpy(refVec, tmpRef, N * sizeof(long));
        memcpy(parVec, tmpPar, N * sizeof(long));
    }
    printf("VERIFICATION %s\n", ok ? "OK" : "FAIL");

    free(refVec); 
    free(parVec); 
    free(tmpRef); 
    free(tmpPar);

    free(inputVector); 
    free(yCsrSerial); 
    free(yCsrParallel);

    free(yDense); 
    free(currentVector);

    freeCSR(csrSerial); 
    freeCSR(csrPar);

    freeDense(D);
    return 0;
}
