#!/bin/bash
gcc -O1 -lrt combine2d.c -o combine2d.exe
#       DELTA ITERS
./combine2d 2 10
./combine2d 2 20
./combine2d 2 40
./combine2d 2 80
./combine2d 2 160
./combine2d 2 320
./combine2d 8 10
./combine2d 8 20
./combine2d 8 40
./combine2d 8 80
./combine2d 8 160
./combine2d 8 320
./combine2d 16 10
./combine2d 16 20
./combine2d 16 40
./combine2d 16 80
./combine2d 16 160
./combine2d 16 320
./combine2d 64 10
./combine2d 64 20
./combine2d 64 40
./combine2d 64 80
./combine2d 64 160
./combine2d 64 320
mv *.csv ./part1_data/
