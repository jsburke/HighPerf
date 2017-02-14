#!/bin/bash

echo "building and running test_branch"
curdir=${PWD##*/}
if [ $curdir != "code" ]
	then
	echo "please run in code directory"
	exit
fi

if [ -d ./p3_data ]
	then
	echo "p3_data directory found"
else
	echo "making directory for data output files"
	mkdir p3_data
fi

echo "compile test_dot.c"
gcc -O1 test_branch.c -lrt -o tbr

#only need to run one test
./tbr

echo "moving data files"
mv *.csv ./p3_data/
rm tbr
echo "test_branch tests complete"
