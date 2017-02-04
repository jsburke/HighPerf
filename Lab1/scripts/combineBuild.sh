#!/bin/bash
#
# This script generates all the data for part 1 of the assignment
#
# To plot output:
#  * Start MATLAB
#  * Using the file navigator ("Current Folder" pane in upper-left) go into
#    the directory HighPerf/Lab1/matlabUtil
#  * In the MATLAB Command Window (bottom-right) type:
#
#        graph_2d('../code/doubleMul_I320_D16.csv');
#
#    (changing the CSV filename as needed)

curdir=${PWD##*/}
if [ ! "$curdir" == "code" ]; then
  echo "This script should be run from within the 'code' directory"
  exit
fi

if [ ! -d "part1_data" ]; then
  mkdir part1_data
fi

gcc -O1 -lrt combine2d.c -o combine2d
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
rm combine2d
