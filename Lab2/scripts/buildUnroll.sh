#!/bin/bash

echo "building and running test_unroll"
curdir=${PWD##*/}
if [ $curdir != "code" ]
	then
	echo "please run in code directory"
	exit
fi

if [ -d ./part1_data ]
	then
	echo "part1_data directory found"
else
	echo "making directory for data output files"
	mkdir part1_data
fi

echo "compile test_unroll.c"
gcc -O1 -lrt test_unroll.c -o unroll

#run a series of tests
#harvest data
./unroll 0 10 50
printf "\n"

./unroll 256 64 50
printf "\n"

./unroll 256 128 50
printf "\n"

./unroll 256 256 30
printf "\n"

echo "moving data files"
mv *.csv ./part1_data/
rm unroll
echo "test_unroll tests complete"