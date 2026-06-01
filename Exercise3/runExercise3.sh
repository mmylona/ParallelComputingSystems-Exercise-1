#!/bin/bash
# =============================================================================
# runExercise3.sh  -  Bank simulation benchmark
# Runs each experiment REPS times and prints the average.
# Also writes a CSV file for use in the report.
# =============================================================================

export LC_ALL=C   # decimal separator fix (Greek locale uses comma)

set -e
cd "$(dirname "$0")"
make -s

REPS=4
THREADS=4
CSV="results.csv"

# ---------- helper -----------------------------------------------------------

# run_avg <label> <args_to_bank...>
# Runs ./bank REPS times, prints the average, appends to CSV.
run_avg() {
    local label="$1"; shift
    local total=0
    for r in $(seq 1 $REPS); do
        local t
        t=$(./bank "$@" | grep "^Parallel time" | awk '{print $4}')
        total=$(echo "$total + $t" | bc -l)
    done
    local avg
    avg=$(echo "$total / $REPS" | bc -l)
    printf "  %-55s  avg = %8.6f s\n" "$label" "$avg"
    printf "%s,%.6f,%s\n" "$label" "$avg" "$*" >> "$CSV"
}

# ---------- CSV header -------------------------------------------------------
echo "label,avg_time_s,bank_args" > "$CSV"

# =============================================================================
echo "=== Exercise 3 - Bank Locking Benchmark (threads=$THREADS, reps=$REPS) ==="

# -----------------------------------------------------------------------------
echo ""
echo "--- 1. Effect of number of accounts (trans/thread=10000, query_pct=50) ---"
for ACCS in 10 100 1000; do
    echo "  accounts = $ACCS"
    for SCHEME in coarse_mutex fine_mutex coarse_rw fine_rw; do
        run_avg "accounts=$ACCS scheme=$SCHEME" \
                $ACCS 10000 50 $SCHEME $THREADS
    done
done

# -----------------------------------------------------------------------------
echo ""
echo "--- 2. Effect of transactions per thread (accounts=100, query_pct=50) ---"
for TRANS in 1000 10000 100000; do
    echo "  trans/thread = $TRANS"
    for SCHEME in coarse_mutex fine_mutex coarse_rw fine_rw; do
        run_avg "trans=$TRANS scheme=$SCHEME" \
                100 $TRANS 50 $SCHEME $THREADS
    done
done

# -----------------------------------------------------------------------------
echo ""
echo "--- 3. Effect of query percentage (accounts=100, trans/thread=10000) ---"
for QPCT in 0 30 70 100; do
    echo "  query_pct = $QPCT%"
    for SCHEME in coarse_mutex fine_mutex coarse_rw fine_rw; do
        run_avg "query_pct=$QPCT scheme=$SCHEME" \
                100 10000 $QPCT $SCHEME $THREADS
    done
done

# -----------------------------------------------------------------------------
echo ""
echo "--- 4. Extended CS: mutex vs rw-lock (accounts=100, trans/thread=1000) ---"
for QPCT in 50 100; do
    echo "  query_pct = $QPCT%"
    for SCHEME in coarse_mutex coarse_rw; do
        for EXT in 0 1; do
            run_avg "query_pct=$QPCT scheme=$SCHEME extended=$EXT" \
                    100 1000 $QPCT $SCHEME $THREADS $EXT
        done
    done
done

# =============================================================================
echo ""
echo "Results saved to: $CSV"