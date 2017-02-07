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
#./blockpose 1 128 3
#./blockpose 1 256 3 
./blockpose 1 512 30
#./blockpose 1 1024 3 
#./blockpose 1 2048 3 
printf "\n"

#./blockpose 2 128 3
#./blockpose 2 256 3 
./blockpose 2 512 30
#./blockpose 2 1024 3 
#./blockpose 2 2048 3 
printf "\n"

#./blockpose 4 128 3
#./blockpose 4 256 3 
./blockpose 4 512 30 
#./blockpose 4 1024 3 
#./blockpose 4 2048 3 
printf "\n"

#./blockpose 8 128 3
#./blockpose 8 256 3 
./blockpose 8 512 30
#./blockpose 8 1024 3 
#./blockpose 8 2048 3 
printf "\n"

#./blockpose 12 128 3
#./blockpose 12 256 3 
./blockpose 12 512 30
#./blockpose 12 1024 3 
#./blockpose 12 2048 3 
printf "\n"

./blockpose 16 512 30
printf "\n"

./blockpose 20 512 30
printf "\n"

./blockpose 25 512 30
printf "\n"

./blockpose 32 512 30
printf "\n"

echo "moving data files"
mv *.csv ./part4_data
rm blockpose
echo "blockpose complete"
