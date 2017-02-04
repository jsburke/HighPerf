#!/bin/bash
gcc -O1 -lrt mmm_inter.c -o mmm_inter
#       BASE DELTA ITERS
./mmm_inter 0 2 2
./mmm_inter 0 2 4
./mmm_inter 0 2 6
./mmm_inter 0 2 8
printf "\n"

./mmm_inter 0 4 2
./mmm_inter 0 4 4
./mmm_inter 0 4 6
./mmm_inter 0 4 8
printf "\n"

./mmm_inter 0 16 2
./mmm_inter 0 16 4
./mmm_inter 0 16 6
./mmm_inter 0 16 8
printf "\n"

./mmm_inter 0 64 2
./mmm_inter 0 64 4
./mmm_inter 0 64 6
./mmm_inter 0 64 8
printf "\n"

./mmm_inter 0 128 2
./mmm_inter 0 128 4
./mmm_inter 0 128 6
./mmm_inter 0 128 8
printf "\n"

./mmm_inter 0 256 2
./mmm_inter 0 256 4
./mmm_inter 0 256 6
./mmm_inter 0 256 8
printf "\n"

mv *.csv ./part2_data/
rm mmm_inter