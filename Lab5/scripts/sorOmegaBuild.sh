#!/bin/bash

echo "building and running test_SOR_OMEGA"
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

echo "compile test_SOR_OMEGA.c"
gcc -O1 -lrt test_SOR_OMEGA.c -o sor_omega

#run a series of tests
#harvest data
./sor_omega 2 8 0.50 20
printf "\n"

./sor_omega 4 16 0.50 20
printf "\n"

./sor_omega 8 32 0.50 20
printf "\n"

echo "moving data files"
mv *.csv ./part1_data/
rm parallel
echo "test_SOR_OMEGA tests complete"