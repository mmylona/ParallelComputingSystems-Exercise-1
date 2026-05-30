#!/bin/bash
set -e
cd "$(dirname "$0")"
make -s

export LC_ALL=C
export LANG=C

echo "=== Exercise 6: Parallel Mergesort ==="
echo ""

REPS=4

run_avg () {
    local N=$1
    local MODE=$2
    local THREADS=$3
    local total=0

    for ((i=1; i<=REPS; i++)); do
        t=$(./mergesort "$N" "$MODE" "$THREADS" | awk '/Sort time/ {print $4}')
        total=$(echo "$total + $t" | bc -l)
    done

    echo "$(echo "scale=6; $total / $REPS" | bc -l)"
}

for N in 10 10000 1000000 10000000 100000000; do

    serial_avg=$(run_avg "$N" serial 1)
    printf "N=%-10d mode=serial    threads=1   avg=%.6f s\n" "$N" "$serial_avg"

    for T in 2 4 8; do
        avg=$(run_avg "$N" parallel "$T")
        speedup=$(echo "scale=4; $serial_avg / $avg" | bc -l)
        printf "N=%-10d mode=parallel  threads=%-3d avg=%.6f s  speedup=%.2fx\n" \
            "$N" "$T" "$avg" "$speedup"
    done

    echo ""
done