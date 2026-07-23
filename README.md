# Table of Contents

- [System Information](#system-information)
- [Execution Instructions](#execution-instructions)
- [Exercise 1: Polynomial Multiplication](#exercise-1-polynomial-multiplication)
  - [1.1 Problem Description](#11-problem-description)
  - [1.2 Solution Description](#12-solution-description)
  - [1.3 Experimental Results](#13-experimental-results)
  - [1.4 Discussion of Results](#14-discussion-of-results)
- [Exercise 2: Contention on a Shared Variable](#exercise-2-contention-on-a-shared-variable)
  - [2.1 Problem Description](#21-problem-description)
  - [2.2 Solution Description](#22-solution-description)
  - [2.3 Experimental Results](#23-experimental-results)
  - [2.4 Discussion of Results](#24-discussion-of-results)
- [Exercise 3: Bank Simulation](#exercise-3-bank-simulation)
  - [3.1 Problem Description](#31-problem-description)
  - [3.2 Solution Description](#32-solution-description)
  - [3.3 Result 1: Effect of Number of Accounts](#33-result-1-effect-of-number-of-accounts)
  - [3.4 Result 2: Effect of Number of Transactions per Thread](#34-result-2-effect-of-number-of-transactions-per-thread)
  - [3.5 Result 3: Effect of Query Percentage](#35-result-3-effect-of-query-percentage)
  - [3.6 Result 4: Extended Critical Section](#36-result-4-extended-critical-section)
  - [3.7 Discussion of Results](#37-discussion-of-results)
- [Exercise 4: Barriers](#exercise-4-barriers)
  - [4.1 Problem Description](#41-problem-description)
  - [4.2 Solution Description](#42-solution-description)
  - [4.3 Experimental Results](#43-experimental-results)
  - [4.4 Discussion of Results](#44-discussion-of-results)
- [Exercise 5: Sparse Matrix-Vector Multiplication](#exercise-5-sparse-matrix-vector-multiplication)
  - [5.1 Problem Description](#51-problem-description)
  - [5.2 Solution Description](#52-solution-description)
  - [5.3 Scaling with Number of Threads](#53-scaling-with-number-of-threads)
  - [5.4 Effect of Sparsity](#54-effect-of-sparsity)
  - [5.5 Effect of Matrix Size](#55-effect-of-matrix-size)
  - [5.6 Effect of Number of Iterations](#56-effect-of-number-of-iterations)
  - [5.7 Discussion of Results](#57-discussion-of-results)
- [Exercise 6: Parallel Mergesort](#exercise-6-parallel-mergesort)
  - [6.1 Problem Description](#61-problem-description)
  - [6.2 Solution Description](#62-solution-description)
  - [6.3 Experimental Results](#63-experimental-results)
  - [6.4 Discussion of Results](#64-discussion-of-results)

## System Information

**Computer system name:** linux01
**Processor model:** Intel Core i5-6500 @ 3.20GHz
**Number of cores:** 4
**Operating system version:** Ubuntu 20.04.6 LTS
**Compiler version:** GCC 9.4.0

## Execution Instructions

To run all exercises, the code is first compiled with the `make` command, and then the programs are executed with the `make run` command.

## Exercise 1: Polynomial Multiplication

### 1.1 Problem Description

This exercise studies the speedup offered by parallel processing through the multiplication of two random polynomials of degree $n$ using the classic $O(n^2)$ algorithm. It requires implementing one serial and two parallel versions (using Pthreads and OpenMP), with dynamic selection of the degree, number of threads, and execution method by the user. The goal is to verify the correctness of the results and compare performance through timing measurements for large problem sizes, in order to evaluate the speedup and the need for synchronization during concurrent writes.

### 1.2 Solution Description

The solution implements polynomial multiplication in such a way that each coefficient of the result is computed independently, eliminating race conditions and the need for synchronization. In the Pthreads version, the positions of the result array are distributed evenly among the threads, while in the OpenMP version dynamic work scheduling is used for better load balancing. In addition, a Bash script automates compilation, running experiments, correctness checking, and logging execution times and speedup to a CSV file for analysis.

### 1.3 Experimental Results

| Model | Degree | Threads | AvgSerial | AvgParallel | AvgSpeedup | Correct |
|---|---|---|---|---|---|---|
| pthreads | 50000 | 1 | 1.434845 | 1.379180 | 1.042500 | YES |
| pthreads | 50000 | 2 | 1.429965 | 0.716533 | 1.995000 | YES |
| pthreads | 50000 | 4 | 1.432403 | 0.554013 | 2.585000 | YES |
| pthreads | 50000 | 8 | 1.432788 | 0.439227 | 3.265000 | YES |
| pthreads | 100000 | 1 | 5.718425 | 5.600000 | 1.020000 | YES |
| pthreads | 100000 | 2 | 5.723850 | 2.885700 | 1.982500 | YES |
| pthreads | 100000 | 4 | 5.722700 | 2.213395 | 2.585000 | YES |
| pthreads | 100000 | 8 | 5.717900 | 1.730200 | 3.305000 | YES |
| openmp | 50000 | 1 | 1.434268 | 1.432843 | 1.002500 | YES |
| openmp | 50000 | 2 | 1.433502 | 0.738545 | 1.940000 | YES |
| openmp | 50000 | 4 | 1.431385 | 0.390008 | 3.670000 | YES |
| openmp | 50000 | 8 | 1.431050 | 0.392037 | 3.650000 | YES |
| openmp | 100000 | 1 | 5.727275 | 5.747525 | 1.000000 | YES |
| openmp | 100000 | 2 | 5.726550 | 2.967000 | 1.927500 | YES |
| openmp | 100000 | 4 | 5.712000 | 1.565640 | 3.647500 | YES |
| openmp | 100000 | 8 | 5.706375 | 1.567143 | 3.642500 | YES |

### 1.4 Discussion of Results

The program generates the polynomials, performs the computations, measures execution times, and verifies that the parallel results are identical to the serial ones. Each coefficient of the final polynomial is computed independently with no data dependencies. For this reason, no synchronization was required, since each thread writes exclusively to its own positions in the result array, avoiding race conditions. The experimental results confirmed the correctness of the implementations and the expected $O(n^2)$ behavior, while significant speedup was observed relative to serial execution, nearly linear up to the number of physical cores of the system. OpenMP and Pthreads showed similar performance, with OpenMP slightly outperforming due to dynamic work distribution and easier thread management via pragmas, while Pthreads offers greater control over thread management. The limited improvements beyond 4 threads indicate that performance is mainly influenced by hardware characteristics, such as the number of physical cores.

## Exercise 2: Contention on a Shared Variable

### 2.1 Problem Description

This exercise studies the problem of contention (race condition) during concurrent access and updates of a shared variable by multiple Pthreads. Specifically, a program is implemented where each thread repeatedly increments a variable initialized to zero, with the goal that the final result be deterministic. To avoid race conditions, three synchronization mechanisms are applied: mutexes, read-write locks, and atomic operations. The goal is a comparative evaluation of the overhead of each approach under different numbers of threads and iterations, in order to draw conclusions about their efficiency and suitability.

### 2.2 Solution Description

The solution implements the safe increment of a shared variable (sharedCounter) by multiple threads, using three synchronization mechanisms: mutexes, read-write locks, and atomic operations. Each thread executes a common function in which it repeatedly increments the variable for a specific number of iterations, with the synchronization method determined dynamically via a selection parameter. Evaluation is performed via a Bash script that runs experiments for different workloads and numbers of threads, repeating each trial to compute the average execution time.

### 2.3 Experimental Results

| Iterations | Threads | Mutex (s) | RWLock (s) | Atomic (s) |
|---|---|---|---|---|
| 10000 | 1 | 0.000284 | 0.000385 | 0.000169 |
| 10000 | 2 | 0.001146 | 0.004151 | 0.001900 |
| 10000 | 4 | 0.005124 | 0.005581 | 0.001481 |
| 10000 | 8 | 0.004487 | 0.047255 | 0.002620 |
| 100000 | 1 | 0.001767 | 0.003025 | 0.000864 |
| 100000 | 2 | 0.007965 | 0.016820 | 0.005404 |
| 100000 | 4 | 0.016843 | 0.038105 | 0.012074 |
| 100000 | 8 | 0.036776 | 0.428545 | 0.025478 |
| 1000000 | 1 | 0.015998 | 0.028486 | 0.007371 |
| 1000000 | 2 | 0.073258 | 0.156309 | 0.055911 |
| 1000000 | 4 | 0.182913 | 0.416507 | 0.126095 |
| 1000000 | 8 | 0.369932 | 3.127375 | 0.250208 |

### 2.4 Discussion of Results

The results show that atomic instructions have the best performance regardless of workload, since they are implemented at the hardware level and exhibit the lowest overhead. In contrast, read-write locks (RWLocks) show the worst performance, particularly in write-heavy workloads, due to increased synchronization cost, while mutexes fall in between. At the same time, negative scaling is observed as the number of threads increases (1 to 8), since execution time increases instead of decreasing. This is due to intense contention for a single shared variable, where the synchronization cost and continuous cache invalidation between processors cancel out the benefits of parallelization.

RWLock: Although it makes sense for read-heavy workloads, here ALL threads write. The rwlock has higher overhead per write than the mutex, because it must update reader/writer counters. The result is 3–8x worse than mutex and 10–100x worse than atomic for large workloads.

It is observed that as threads and iterations increase, execution time increases almost linearly for mutex/atomic (due to contention), while for rwlock the increase is exponential due to the increased overhead.

## Exercise 3: Bank Simulation

### 3.1 Problem Description

This exercise simulates a multithreaded banking system using Pthreads and focuses on studying synchronization strategies on a shared account array. Threads perform concurrent money transfer transactions and balance queries, with the main goal of comparatively evaluating coarse-grained versus fine-grained synchronization techniques, as well as mutexes versus rw-locks. The analysis examines performance under different workloads, read/write ratios, and numbers of accounts, while also investigating the effect of artificial delay on reads, in order to highlight the advantages of rw-locks in read-heavy scenarios.

### 3.2 Solution Description

To avoid race conditions, the solution implements coarse-grained and fine-grained synchronization using mutexes and read-write locks. In the coarse-grained model, a single common lock is used for the entire array, simplifying management but increasing contention, while in the fine-grained model separate locks per account are applied with ordering based on ID to avoid deadlocks, allowing greater parallelism at the cost of increased overhead. Additionally, RWLocks allow concurrent reads and are preferred in read-heavy scenarios. Correctness is ensured by checking that the total capital remains constant, and performance evaluation is automated with a Bash script that varies the number of accounts, the read/write ratio, and critical section delays.

### 3.3 Result 1: Effect of Number of Accounts

(4 threads, 10000 transactions/thread, 50% queries)

| Accounts | Coarse Mutex (s) | Fine Mutex (s) | Coarse RW (s) | Fine RW (s) |
|---|---|---|---|---|
| 10 | 0.002944 | 0.007816 | 0.009454 | 0.015617 |
| 100 | 0.003215 | 0.002118 | 0.008396 | 0.002737 |
| 1000 | 0.003104 | 0.001402 | 0.014994 | 0.002819 |

### 3.4 Result 2: Effect of Number of Transactions per Thread

(4 threads, 100 accounts, 50% queries)

| Transactions/thread | Coarse Mutex (s) | Fine Mutex (s) | Coarse RW (s) | Fine RW (s) |
|---|---|---|---|---|
| 1000 | 0.000744 | 0.000682 | 0.001062 | 0.000710 |
| 10000 | 0.005901 | 0.007519 | 0.032788 | 0.010040 |
| 100000 | 0.050309 | 0.019403 | 0.083143 | 0.024520 |

### 3.5 Result 3: Effect of Query Percentage

(4 threads, 100 accounts, 10000 transactions/thread)

| % Queries | Coarse Mutex (s) | Fine Mutex (s) | Coarse RW (s) | Fine RW (s) |
|---|---|---|---|---|
| 0% | 0.003445 | 0.003213 | 0.004263 | 0.003334 |
| 30% | 0.003167 | 0.002786 | 0.009515 | 0.010929 |
| 70% | 0.010165 | 0.005944 | 0.036248 | 0.008106 |
| 100% | 0.007267 | 0.003519 | 0.013972 | 0.003395 |

### 3.6 Result 4: Extended Critical Section (Extended CS)

(4 threads, 100 accounts, 1000 transactions/thread)

| % Queries | Scheme | Extended=0 (s) | Extended=1 (s) |
|---|---|---|---|
| 50% | Coarse Mutex | 0.001135 | 0.152101 |
| 50% | Coarse RW | 0.001968 | 0.161135 |
| 100% | Coarse Mutex | 0.000891 | 0.292562 |
| 100% | Coarse RW | 0.001280 | 0.075155 |

### 3.7 Discussion of Results

The experimental results show that performance depends strongly on system size and workload type. With few accounts (e.g., n=10), the coarse-grained model is slightly better, since fine-grained introduces overhead without substantial parallelism gains due to high contention. In contrast, on larger systems (n=1000), fine-grained is significantly better, since it reduces conflicts and allows better scaling. Regarding synchronization type, RWLocks are generally slower than mutexes due to increased OS overhead, and perform competitively only in scenarios with exclusively reads and sufficiently "heavy" critical sections, where the ability to have concurrent readers becomes genuinely usable.

As the duration of the read critical section increases, RWLocks significantly outperform mutexes in read-heavy workloads, since they allow multiple readers to execute concurrently, while mutexes continue to serialize all accesses.

## Exercise 4: Barriers

### 4.1 Problem Description

This exercise concerns the implementation and comparative evaluation of three barrier mechanisms for synchronizing multiple threads (Pthreads) in a loop with many iterations. Specifically, the built-in `pthread_barrier`, a custom barrier using a mutex and condition variable without busy-waiting, and a sense-reversal barrier based on busy-waiting are implemented. The goal is to analyze performance under different workloads and oversubscription conditions, as well as to understand the efficient and safe reusability of the sense-reversal barrier despite the threads continuously waiting.

### 4.2 Solution Description

The solution implements the three barrier approaches using compile-time macros to select the implementation. First, `pthread_barrier` is used as a reference point. In the second approach (CondBarrier), the barrier is implemented with a mutex and condition variable, using a cycle variable to ensure correct reuse and avoid deadlock between successive phases. The third approach (SenseBarrier) is based on the sense-reversal algorithm using atomic operations and busy-waiting, where each thread toggles a local sense variable and waits until it matches the global sense, ensuring correct synchronization between iterations. The evaluation script runs experiments for different numbers of threads and iterations, avoiding SenseBarrier in oversubscription cases, since busy-waiting leads to wasted CPU time and significant performance degradation.

### 4.3 Experimental Results

| Type | Threads | 1000 iters (s) | 10000 iters (s) | 100000 iters (s) |
|---|---|---|---|---|
| pthread | 1 | 0.003796 | 0.015641 | 0.091550 |
| condvar | 1 | 0.000300 | 0.000306 | 0.001975 |
| sense | 1 | 0.000262 | 0.000188 | 0.000913 |
| pthread | 2 | 0.003071 | 0.021734 | 0.219116 |
| condvar | 2 | 0.004440 | 0.037044 | 0.362935 |
| sense | 2 | 0.000238 | 0.001218 | 0.010875 |
| pthread | 4 | 0.005078 | 0.042510 | 0.485405 |
| condvar | 4 | 0.007605 | 0.181160 | 0.698235 |
| sense | 4 | 0.001935 | 0.007440 | 0.020467 |
| pthread | 8 | 0.009874 | 0.094964 | 0.933458 |
| condvar | 8 | 0.023091 | 0.181237 | 1.745052 |
| sense | 8 | SKIPPED | SKIPPED | SKIPPED |
| pthread | 16 | 0.020064 | 0.187952 | 1.959448 |
| condvar | 16 | 0.055664 | 0.387715 | 3.555825 |
| sense | 16 | SKIPPED | SKIPPED | SKIPPED |

### 4.4 Discussion of Results

Overall, `pthread_barrier` and the mutex + condvar implementation show more stable and "smooth" behavior, with the former usually being faster due to its optimized implementation, while the sense-reversal barrier is the fastest when the number of threads does not exceed the number of cores, but degrades sharply under oversubscription due to busy-waiting. When the number of threads exceeds the number of cores, all schemes are burdened by context switching, but condvar/pthread-barrier simply slow down, while sense-reversal can collapse due to continuous CPU consumption. Sense-reversal is reusable because it uses a local sense variable in each thread that toggles at each phase and is compared to a shared global value, allowing correct synchronization across successive rounds of the barrier.

## Exercise 5: Sparse Matrix-Vector Multiplication

### 5.1 Problem Description

The exercise requires implementing a program in OpenMP for efficient processing of sparse matrices, specifically the construction of a random square matrix and its conversion to CSR (Compressed Sparse Row) format, as well as multiplying the matrix by a vector for multiple iterations. The program implements both serial and parallel versions of the steps (CSR construction and matrix-vector multiplication), in order to compare their performance based on execution time and number of threads. In addition, a comparison is made between the CSR representation and the classic dense representation in terms of speed and efficiency, taking into account different matrix sizes, sparsity percentages, and number of iterations, with the goal of evaluating the scaling and advantages of parallelization and the use of CSR.

### 5.2 Solution Description

First, a random dense square matrix with a predetermined sparsity percentage and an input vector are created. Then, the matrix is converted, both serially and in parallel, to CSR format, storing only the non-zero values, their corresponding columns, and row pointers, in order to reduce memory usage and computational load. This is followed by matrix-vector multiplication (SpMV) in three versions: serial CSR, parallel CSR with OpenMP, and parallel dense format, for performance comparison across multiple iterations where the result of each step becomes the input of the next. Finally, execution times are measured for all stages, and correctness is verified between the serial and parallel implementations, in order to evaluate both the parallelization speedup and the benefit of CSR versus dense representation.

### 5.3 Scaling with Number of Threads

Parameters: N = 2000, sparsity = 90%, iters = 10, threads = {1, 2, 4, 8}.

| Threads | CSR Build (s) | CSR SpMV Serial (s) | CSR SpMV Par (s) | Dense SpMV Par (s) | Speedup SpMV |
|---|---|---|---|---|---|
| 1 | 0.01091 | 0.00275 | 0.00273 | 0.02503 | 1.007 |
| 2 | 0.00610 | 0.00278 | 0.00154 | 0.01384 | 1.811 |
| 4 | 0.00394 | 0.00293 | 0.00096 | 0.01178 | 3.064 |
| 8 | 0.00463 | 0.00276 | 0.00186 | 0.01321 | 1.481 |


Observations: Parallel CSR SpMV scales satisfactorily from 1 to 4 threads (speedup ≈ 3.06x). At 8 threads, the speedup drops to 1.48x, likely due to contention and thread-management overhead on a machine with 4 physical cores (hyper-threading). Parallel Dense SpMV benefits less from parallelism compared to CSR, since the higher number of non-zero operations increases pressure on the cache.

### 5.4 Effect of Sparsity

Parameters: N = 2000, threads = 4, iters = 10, sparsity ∈ {0, 25, 50, 70, 80, 90, 95, 99}%.

| Sparsity (%) | NNZ | Build (s) | CSR SpMV (s) | Dense SpMV (s) | CSR Total (s) |
|---|---|---|---|---|---|
| 0 | 4,000,000 | 0.00852 | 0.01731 | 0.01197 | 0.02583 |
| 25 | 3,000,119 | 0.00884 | 0.01291 | 0.01194 | 0.02175 |
| 50 | 1,998,332 | 0.00879 | 0.00838 | 0.01192 | 0.01716 |
| 70 | 1,197,700 | 0.00794 | 0.00451 | 0.01200 | 0.01245 |
| 80 | 798,216 | 0.00562 | 0.00266 | 0.01217 | 0.00828 |
| 90 | 398,755 | 0.00395 | 0.00096 | 0.01175 | 0.00491 |
| 95 | 199,398 | 0.00318 | 0.00041 | 0.01165 | 0.00359 |
| 99 | 40,137 | 0.00266 | 0.00013 | 0.01139 | 0.00278 |

Observations: CSR SpMV time decreases linearly as sparsity increases, since the number of non-zero elements decreases proportionally. In contrast, Dense SpMV maintains an almost constant time (~0.012s) regardless of sparsity, since it always processes N² elements. The total CSR time (build + SpMV) becomes smaller than Dense SpMV for sparsity > ~70%, indicating that the CSR representation is worthwhile for sufficiently sparse matrices.

### 5.5 Effect of Matrix Size

Parameters: sparsity = 90%, threads = 4, iters = 5, N ∈ {500, 1000, 2000, 3000, 5000}.

| N | CSR Build (s) | CSR SpMV (s) | Dense SpMV (s) | CSR Total (s) |
|---|---|---|---|---|
| 500 | 0.000307 | 0.000036 | 0.000175 | 0.000343 |
| 1000 | 0.001018 | 0.000102 | 0.001203 | 0.001120 |
| 2000 | 0.003964 | 0.000580 | 0.005867 | 0.004544 |
| 3000 | 0.008565 | 0.001705 | 0.013737 | 0.010270 |
| 5000 | 0.023200 | 0.005319 | 0.038440 | 0.028519 |

Observations: Both methods show O(N²) scaling with respect to total time, but CSR scales much more favorably due to the 90% sparsity. For N = 5000, Dense SpMV requires ~38ms while CSR (build + SpMV) requires only ~28.5ms, highlighting a clear advantage of the CSR representation for larger sizes. For small N (≤ 1000), Dense SpMV can be competitive due to low cache-miss overhead.

### 5.6 Effect of Number of Iterations

Parameters: N = 2000, sparsity = 90%, threads = 4, iters = {1, 3, 5, 8, 10, 15, 20}.

| Iterations | CSR Build (s) | CSR SpMV (s) | Dense SpMV (s) | CSR Total (s) |
|---|---|---|---|---|
| 1 | 0.003946 | 0.000128 | 0.001247 | 0.004074 |
| 3 | 0.005789 | 0.000394 | 0.003532 | 0.006183 |
| 5 | 0.003968 | 0.000569 | 0.005878 | 0.004537 |
| 8 | 0.003993 | 0.000815 | 0.009556 | 0.004808 |
| 10 | 0.003952 | 0.000955 | 0.012218 | 0.004907 |
| 15 | 0.003963 | 0.001315 | 0.017567 | 0.005278 |
| 20 | 0.003960 | 0.001683 | 0.023076 | 0.005643 |

Observations: CSR build time (~0.004s) remains constant regardless of the number of iterations, since it is performed once before the loop. CSR SpMV time increases linearly and slowly, while Dense SpMV increases more rapidly. The crossover point is located at approximately 4 iterations: beyond this, the total CSR time (build + SpMV) is smaller than Dense SpMV, confirming that the cost of building the CSR is "amortized" quickly.

### 5.7 Discussion of Results

The experimental results show that the CSR representation combined with parallelization via OpenMP is a particularly efficient solution for sparse matrix-vector multiplication (SpMV). The parallel implementation shows significant speedup, reaching approximately triple performance with 4 threads, while further increasing to 8 threads does not lead to a corresponding improvement due to the additional cost of hyper-threading. At the same time, the CSR representation outperforms the dense one when the sparsity percentage exceeds 70%, offering better memory utilization and shorter execution times, particularly for large matrices. Furthermore, the cost of building the CSR structure is amortized after only a few iterations, making it efficient even in applications with a limited number of computations. Overall, the use of CSR with OpenMP is recommended for medium and large sparse matrices, as it ensures high performance and resource economy.

## Exercise 6: Parallel Mergesort

### 6.1 Problem Description

The exercise requires implementing merge sort in C/C++ using OpenMP and the `task` directive, in order to achieve parallelization of the top-down version of the algorithm. The program takes as input the size of the integer array, the choice of serial or parallel sorting execution, and the number of threads, generates a random but deterministic array, and then sorts it. During parallel execution, the algorithm is split into tasks for the sorting and merging sub-problems, leveraging the available threads for speedup. Finally, a correctness check verifies that the array is sorted, and execution time is measured, with the goal of comparing serial and parallel performance for different input sizes and evaluating the speedup offered by parallelism.

### 6.2 Solution Description

The solution implements top-down merge sort by converting the serial recursive algorithm into a parallel one via the OpenMP interface. Initially, a single thread (`#pragma omp single`) starts the recursion for the entire array, creating new independent sub-problems (left and right halves) via the `#pragma omp task` directive. The critical optimization point is the use of the condition `if(size > TASK_THRESHOLD)`: this condition prevents OpenMP from creating unnecessary, delayed tasks when the size of the sub-array becomes too small (e.g., below 100,000 elements), forcing the threads to execute the small recursions serially and thereby eliminating the disproportionate overhead of thread management. Finally, `#pragma omp taskwait` guarantees that the merge will start only after both halves have been fully sorted, while an automated Bash script runs the program for huge arrays (up to $10^8$ elements) and different numbers of threads, directly extracting the average times and the resulting speedup.

### 6.3 Experimental Results

| N | Mode | Threads | Avg (s) | Speedup |
|---|---|---|---|---|
| 10 | serial | 1 | 0.000001 | --- |
| 10 | parallel | 2 | 0.000070 | 0.01x |
| 10 | parallel | 4 | 0.001035 | 0.00x |
| 10 | parallel | 8 | 0.000259 | 0.00x |
| 10000 | serial | 1 | 0.000826 | --- |
| 10000 | parallel | 2 | 0.001287 | 0.64x |
| 10000 | parallel | 4 | 0.002744 | 0.30x |
| 10000 | parallel | 8 | 0.001496 | 0.55x |
| 1000000 | serial | 1 | 0.095602 | --- |
| 1000000 | parallel | 2 | 0.123662 | 0.77x |
| 1000000 | parallel | 4 | 0.066962 | 1.43x |
| 1000000 | parallel | 8 | 0.042516 | 2.25x |
| 10000000 | serial | 1 | 1.088790 | --- |
| 10000000 | parallel | 2 | 1.385468 | 0.79x |
| 10000000 | parallel | 4 | 0.743151 | 1.47x |
| 10000000 | parallel | 8 | 0.457230 | 2.38x |
| 100000000 | serial | 1 | 12.383369 | --- |
| 100000000 | parallel | 2 | 15.349740 | 0.81x |
| 100000000 | parallel | 4 | 8.233002 | 1.50x |
| 100000000 | parallel | 8 | 4.915604 | 2.52x |

### 6.4 Discussion of Results

The results show that parallelizing top-down mergesort with OpenMP tasks provides no benefit for small arrays (N ≤ 10⁴), since the cost of creating tasks and synchronization exceeds the computational work, resulting in even a slowdown compared to serial execution. In contrast, for larger sizes (N ≥ 10⁶), a clear speedup appears, which increases with the number of threads and reaches approximately 2.5x with 8 threads on the largest instances (N = 10⁸).

The limited scaling relative to the ideal speedup is due to the fact that part of the algorithm remains serial (mainly the merge phase), as well as the overhead of task management. The `if()` condition in the task directive is critical, because it prevents the creation of excessively small tasks that disproportionately increase overhead instead of offering parallelism, thereby noticeably improving performance for large N.
