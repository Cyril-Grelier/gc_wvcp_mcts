#!/bin/bash

echo "dont exec this file"
exit 1

# steps to runs all jobs

# 0 : set wanted instances, random seed, method, ... in to_eval_generator.py
./scripts/build.sh
# or
srun --partition=SMP-short --exclude=cribbar[041-056] --time=00:10:00 ./scripts/build.sh


# 1 : generate folders and list of commands
python3 scripts/generator_to_eval.py

# 2 : split the to_eval file
split -l 1000 -d to_eval to_eval

# 3 : launch each jobs by 1000 and add the job id after launching each command
attention au nombre de lignes dans to_eval !!!!
modifier dans one_job_slurm #SBATCH --array=1-1000 (1 et 1000 inclus)

sbatch scripts/one_job_slurm_algo_mem.sh to_eval

sbatch scripts/one_job_slurm.sh to_eval

# 4 : check for problems
find output_slurm/ -size 0 -delete
find output_slurm/ -ls -exec cat {} \;
find output_test_slurm -name "*.csv.running" -ls
