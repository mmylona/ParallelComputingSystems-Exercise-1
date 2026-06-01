#!/bin/bash
set -e
cd "$(dirname "$0")"
make -s

echo "=== Exercise 2 ==="
echo ""

REPS=4
for ITERS in 10000 100000 1000000; do
    for T in 1 2 4 8; do
        for MODE in mutex rwlock atomic; do
            total=0
            for r in $(seq 1 $REPS); do
                t=$(./sharedCounter $T $ITERS $MODE | awk '{print $1}')
                total=$(LC_ALL=C awk "BEGIN {print $total + $t}")
            done
            avg=$(LC_ALL=C awk "BEGIN {printf \"%.6f\", $total / $REPS}")
            printf "iters=%-8d threads=%-3d mode=%-8s avg=%s s\n" \
                   $ITERS $T $MODE $avg
        done
    done
done