#!/bin/bash

echo "building and running blockpose"
curdir=${PWD##*/}
if [ $curdir != "code" ]
	then
	echo "please run in code directory"
	exit
fi

if [ -d ./part4_data ]
	then
	echo "part4_data directory found"
else
	echo "making directory for data output files"
	mkdir part4_data
fi

echo "compile blockpose"
gcc -O1 -lrt test_transpose_block.c -o blockpose

# Each of these really deserves reevaluation based on the 
# data type
# 8 should be good for normal floats if I'm not too tired atm
echo "run tests"
./blockpose 1 128 3
./blockpose 1 256 3 
./blockpose 1 512 3 
./blockpose 1 1024 3 
printf "\n"

./blockpose 2 128 3
./blockpose 2 256 3 
./blockpose 2 512 3 
./blockpose 2 1024 3 
printf "\n"

./blockpose 4 128 3
./blockpose 4 256 3 
./blockpose 4 512 3 
./blockpose 4 1024 3 
printf "\n"

mv *.csv ./part4_data
rm blockpose
echo "blockpose complete"