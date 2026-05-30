#!/bin/bash
# run_ex4.sh
#
# Runs all three barrier implementations across a range of thread
# counts and iteration counts, repeating each experiment REPS times
# and reporting the average wall-clock time.
#
# Results are written to results.csv and printed to stdout.
#
# Usage:  bash run_ex4.sh
#
# Important: The sense-reversal barrier busy-waits.  When the number
# of threads exceeds the number of physical cores, threads that are
# not scheduled cannot make progress, so the spinning threads waste
# their time-slice and throughput collapses.  The script runs the
# sense barrier only when T <= nproc.

set -euo pipefail
cd "$(dirname "$0")"
make -s

REPS=4
NCORES=$(nproc)

THREAD_COUNTS="1 2 4 8 16"
ITER_COUNTS="1000 10000 100000"

CSV="results.csv"
echo "type,threads,iterations,avg_time_s" > "$CSV"

echo "=== Exercise 1.4: Barrier Benchmarks ==="
echo "Cores available : $NCORES"
echo "Repetitions     : $REPS"
echo ""

# Helper: run one binary $REPS times and record the average.
# Output line format:  "pthread_barrier : 0.014328 s"
# $(NF-1) extracts the numeric field (second-to-last, before the "s").
run_bench() {
    local type="$1"
    local T="$2"
    local ITERS="$3"
    local bin="./barrier_${type}"

    local total=0
    local t
    local i
    for i in $(seq 1 "$REPS"); do
        t=$("$bin" "$T" "$ITERS" 2>/dev/null \
            | awk '/barrier/ {print $(NF-1); exit}')
        total=$(awk "BEGIN{print $total + $t}")
    done

    local avg
    avg=$(awk "BEGIN{printf \"%.6f\", $total / $REPS}")
    printf "type=%-8s threads=%-3d iters=%-8d avg=%s s\n" \
           "$type" "$T" "$ITERS" "$avg"
    echo "${type},${T},${ITERS},${avg}" >> "$CSV"
}

for T in $THREAD_COUNTS; do
    for ITERS in $ITER_COUNTS; do
        run_bench pthread "$T" "$ITERS"
        run_bench condvar "$T" "$ITERS"

        if [ "$T" -le "$NCORES" ]; then
            run_bench sense "$T" "$ITERS"
        else
            printf "type=%-8s threads=%-3d iters=%-8d SKIPPED (T > %d cores, busy-wait stalls)\n" \
                   "sense" "$T" "$ITERS" "$NCORES"
            echo "sense,${T},${ITERS},N/A" >> "$CSV"
        fi
    done
    echo ""
done

echo "Results saved to $CSV"