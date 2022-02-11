#!/bin/bash

# get the command line then eval it
line=$(head -n "$1" $2 | tail -n 1)

cd build_release || exit

eval "$line"

# once the job is done, check the solution

# get the output file, last part of the command line
file=$(echo "$line" | rev | cut -d" " -f1 | rev)

cd ../scripts || exit

./solution_checker.sh "$file"