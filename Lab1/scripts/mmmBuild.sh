#!/bin/bash
gcc -O1 -lrt mmm_inter.c -o mmm_inter
#       BASE DELTA ITERS
./mmm_inter 0 2 2
./mmm_inter 0 2 4
./mmm_inter 0 2 6
./mmm_inter 0 2 8
./mmm_inter 0 2 10
./mmm_inter 0 2 12

./mmm_inter 0 4 2
./mmm_inter 0 4 4
./mmm_inter 0 4 6
./mmm_inter 0 4 8
./mmm_inter 0 4 10
./mmm_inter 0 4 12

./mmm_inter 0 16 2
./mmm_inter 0 16 4
./mmm_inter 0 16 6
./mmm_inter 0 16 8
./mmm_inter 0 16 10
./mmm_inter 0 16 12

./mmm_inter 0 64 2
./mmm_inter 0 64 4
./mmm_inter 0 64 6
./mmm_inter 0 64 8
./mmm_inter 0 64 10
./mmm_inter 0 64 12
mv *.csv ./part2_data/
rm mmm_inter