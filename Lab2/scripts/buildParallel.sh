#!/bin/bash

echo "building and running test_parallel"
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

echo "compile test_parallel.c"
gcc -O1 -lrt test_parallel.c -o parallel

#run a series of tests
#harvest data
./parallel 0 10 50
printf "\n"

./parallel 256 64 50
printf "\n"

./parallel 256 128 50
printf "\n"

./parallel 256 256 30
printf "\n"

echo "moving data files"
mv *.csv ./part1_data/
rm parallel
echo "test_parallel tests complete"