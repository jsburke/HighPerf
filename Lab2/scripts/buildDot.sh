#!/bin/bash

echo "building and running test_dot"
curdir=${PWD##*/}
if [ $curdir != "code" ]
	then
	echo "please run in code directory"
	exit
fi

if [ -d ./p2_data ]
	then
	echo "p2_data directory found"
else
	echo "making directory for data output files"
	mkdir p2_data
fi

echo "compile test_dot.c"
gcc -O1 test_dot.c -lrt -o tdt

#only need to run one test
./tdt

echo "moving data files"
mv *.csv ./p2_data/
rm tdt
echo "test_dot tests complete"
