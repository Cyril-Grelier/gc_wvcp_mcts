#!/bin/bash

#SBATCH --job-name=gc_wvcp
#SBATCH --nodes=1
#SBATCH --ntasks-per-node=1
#SBATCH --array=1-1000
#SBATCH --time=01:10:00
#SBATCH --partition=SMP-short
#SBATCH --exclude=cribbar[041-056]
#SBATCH --output=slurm_output/slurm-%x-%a-%j.out
#SBATCH --mem=8G

# get the command line then eval it in build directory
line=$(head -n "${SLURM_ARRAY_TASK_ID}" $1 | tail -n 1)

cd build_release || exit

eval "$line"

# once the job is done, check the solution

# get the output file, last part of the command line
file=$(echo "$line" | rev | cut -d" " -f1 | rev)

cd ../scripts || exit

./solution_checker.sh "$file"
