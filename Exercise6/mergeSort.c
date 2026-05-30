#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

#define TASK_THRESHOLD 100000

static void merge(int *arr, int *tmp, int left, int mid, int right) {
    int i = left, j = mid, k = left;

    while (i < mid && j < right) {
        if (arr[i] <= arr[j]) tmp[k++] = arr[i++];
        else                  tmp[k++] = arr[j++];
    }

    while (i < mid)   tmp[k++] = arr[i++];
    while (j < right) tmp[k++] = arr[j++];

    memcpy(arr + left, tmp + left, (right - left) * sizeof(int));
}

static void mergesortSerial(int *arr, int *tmp, int left, int right) {
    if (right - left <= 1) return;

    int mid = left + (right - left) / 2;

    mergesortSerial(arr, tmp, left, mid);
    mergesortSerial(arr, tmp, mid, right);

    merge(arr, tmp, left, mid, right);
}

static void mergesortParallel(int *arr, int *tmp, int left, int right) {
    if (right - left <= 1) return;

    int size = right - left;
    int mid  = left + size / 2;

    /* if() clause: δημιούργησε task ΜΟΝΟ αν το τμήμα είναι αρκετά μεγάλο.
       Αλλιώς ο OpenMP εκτελεί τον κώδικα inline (χωρίς task overhead). */
    #pragma omp task shared(arr, tmp) if(size > TASK_THRESHOLD)
    mergesortParallel(arr, tmp, left, mid);

    #pragma omp task shared(arr, tmp) if(size > TASK_THRESHOLD)
    mergesortParallel(arr, tmp, mid, right);

    #pragma omp taskwait

    merge(arr, tmp, left, mid, right);
}

static int isSorted(int *arr, int n) {
    for (int i = 1; i < n; i++)
        if (arr[i] < arr[i - 1]) return 0;
    return 1;
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <size> <serial|parallel> <num_threads>\n", argv[0]);
        return 1;
    }

    int n          = atoi(argv[1]);
    int use_serial = (argv[2][0] == 's');
    int nthreads   = atoi(argv[3]);

    int *arr = malloc(n * sizeof(int));
    int *tmp = malloc(n * sizeof(int));

    srand(1234); /* deterministic input */
    for (int i = 0; i < n; i++)
        arr[i] = rand();

    double t0 = omp_get_wtime();

    if (use_serial) {
        mergesortSerial(arr, tmp, 0, n);
    } else {
        #pragma omp parallel num_threads(nthreads)
        {
            #pragma omp single
            {
                /* Ένα αρχικό task για όλο τον πίνακα — το single εγγυάται
                   ότι μόνο ένα thread ξεκινά την αναδρομή. */
                #pragma omp task
                mergesortParallel(arr, tmp, 0, n);

                #pragma omp taskwait
            }
        }
    }

    double elapsed = omp_get_wtime() - t0;

    printf("Array size      : %d\n", n);
    printf("Mode            : %s\n", use_serial ? "serial" : "parallel");
    printf("Threads         : %d\n", nthreads);
    printf("Sort time       : %.6f s\n", elapsed);
    printf("Sorted          : %s\n", isSorted(arr, n) ? "YES" : "NO");

    free(arr);
    free(tmp);
    return 0;
}