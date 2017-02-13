#!/bin/bash

echo "building and running blockpose"
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

echo "compile test_combine1-7.c"
gcc -O1 -lrt test_combine1-7.c -o test_combine1-7

#run a series of tests
#harvest data
./test_combine1-7 0 10 50
printf "\n"

./test_combine1-7 0 30 50
printf "\n"

./test_combine1-7 100 10 50
printf "\n"

./test_combine1-7 250 30 50
printf "\n"

echo "moving data files"
mv *.csv ./part1_data/
rm test_combine1-7
echo "test_combine1-7 tests complete"
