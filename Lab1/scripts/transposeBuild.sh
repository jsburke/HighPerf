#!/bin/bash

echo "building and running transpose"
curdir=${PWD##*/}
if [! "$curdir" == "code" ]; then
	echo "please run in code directory"
	exit
fi

if [! -d "part3_data" ];then
	echo "making directory for data output files"
	mkdir part3_data
fi

echo "compile transpose"
gcc -O1 -lrt test_transpose.c -o test_transpose

# following will need updates once blocking is implemented
echo "run tests"
./transpose 0 64 5
./transpose 0 64 10
./transpose 0 64 15
./transpose 0 64 25
printf "\n"

./transpose 0 128 5
./transpose 0 128 10
./transpose 0 128 15
./transpose 0 128 25
printf "\n"

./transpose 0 256 5
./transpose 0 256 10
#ending here since the last on takes ~ 6 mins on laptop
#will probably be more tolerable on lab machines
printf "\n"

mv *.csv ./part3_data
rm transpose
echo "transpose complete"