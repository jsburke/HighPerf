#!/bin/bash
gcc -O1 -lrt combine2d.c -o combine2d
./combine2d
mv *.csv ./p1_data/
