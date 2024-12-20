#!/bin/bash

# Navigate to the project directory
cd build

# Build the project
cmake ..
cmake --build .

for r in $(seq 1 10)
do
    ./JOBASP-f --in bm_la --cap 6 --N 10 --repl $r --eval 1
    ./JOBASP-f --in bm_la --cap 6 --N 10 --repl $r --eval 0
done
