#!/bin/bash

nb_tasks=$(wc -l < $1)

tasks_range=$(seq 1 $nb_tasks)

nb_jobs=10

parallel --jobs "$nb_jobs" ./scripts/one_job_parallel.sh "{1}" $1 "2>>" errors.err ";" ::: "${tasks_range[@]}"
