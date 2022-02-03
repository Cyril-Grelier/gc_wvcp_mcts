#!/bin/bash

# get the command line then eval it
line=$(head -n "$1" to_eval | tail -n 1)

cd build_release || exit

eval "$line"

# once the job is done, check the solution
file=$(echo "$line" | rev | cut -d" " -f1 | rev)

./../scripts/solution_checker.sh "$file"
