#!/bin/bash
cd "$(dirname "$0")"

# Compile
gcc -O3 -fopenmp multiplicationOfPolynomials.c ../utils.c -o multiplicationOfPolynomials -lpthread -lm

# Configuration
DEGREES=(50000 100000)
THREADS=(1 2 4 8)
MODELS=("pthreads" "openmp")
REPEATS=4
OUT="./results.csv"

echo "Model,Degree,Threads,AvgSerial,AvgParallel,AvgSpeedup,Correct" > $OUT

for m in "${MODELS[@]}"; do
    for n in "${DEGREES[@]}"; do
        for t in "${THREADS[@]}"; do
            sum_s=0; sum_p=0; sum_sp=0; status="YES"

            echo "Testing $m | N=$n | Threads=$t"

            for ((r=1; r<=REPEATS; r++)); do
                output=$(./multiplicationOfPolynomials -a $n -b $n -t $t -m $m)
                s=$(echo "$output" | awk '{print $1}')
                p=$(echo "$output" | awk '{print $2}')
                sp=$(echo "$output" | awk '{print $3}')
                ok=$(echo "$output" | awk '{print $4}')

                sum_s=$(awk -v a="$sum_s" -v b="$s"   'BEGIN {print a + b}')
                sum_p=$(awk -v a="$sum_p" -v b="$p"   'BEGIN {print a + b}')
                sum_sp=$(awk -v a="$sum_sp" -v b="$sp" 'BEGIN {print a + b}')
                if [ "$ok" != "YES" ]; then status="NO"; fi
            done

            avg_s=$(awk  -v s="$sum_s"  -v r="$REPEATS" 'BEGIN {printf "%.6f", s / r}')
            avg_p=$(awk  -v s="$sum_p"  -v r="$REPEATS" 'BEGIN {printf "%.6f", s / r}')
            avg_sp=$(awk -v s="$sum_sp" -v r="$REPEATS" 'BEGIN {printf "%.6f", s / r}')

            echo "$m,$n,$t,$avg_s,$avg_p,$avg_sp,$status" >> $OUT
        done
    done
done

echo "Benchmarking complete. Data saved to $OUT"