#!/bin/bash

# Compile
gcc -O3 -fopenmp multiplicationOfPolynomials.c -o multiplicationOfPolynomials -lpthread

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
                # Run program and capture output
                read s p sp ok <<< $(./multiplicationOfPolynomials -a $n -b $n -t $t -m $m)
                
                sum_s=$(echo "$sum_s + $s" | bc -l)
                sum_p=$(echo "$sum_p + $p" | bc -l)
                sum_sp=$(echo "$sum_sp + $sp" | bc -l)
                if [ "$ok" != "YES" ]; then status="NO"; fi
            done
            
            # Calculate Averages
            avg_s=$(echo "$sum_s / $REPEATS" | bc -l)
            avg_p=$(echo "$sum_p / $REPEATS" | bc -l)
            avg_sp=$(echo "$sum_sp / $REPEATS" | bc -l)
            
            echo "$m,$n,$t,$avg_s,$avg_p,$avg_sp,$status" >> $OUT
        done
    done
done
echo "Benchmarking complete. Data saved to $OUT"