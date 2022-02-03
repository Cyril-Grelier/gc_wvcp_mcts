#!/bin/bash

nb_tasks=$(wc -l < to_eval)

tasks_range=$(seq 1 $nb_tasks)

nb_jobs=10

parallel --jobs "$nb_jobs" ./one_job.sh "{1}" "2>>" errors.err ";" ::: "${tasks_range[@]}"
