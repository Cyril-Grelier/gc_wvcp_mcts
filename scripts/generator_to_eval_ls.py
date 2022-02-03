"""
Generate to_eval file which list all run to perform for local search

to split :
    split -l 1000 -d to_eval_ls to_eval_ls

instance                  // i,instance
method                    // m,method
time_limit                // t,time_limit
rand_seed                 // r,rand_seed
target                    // T,target
initialization_str        // I,initialization
nb_iter_local_search      // N,nb_iter_local_search
max_time_local_search     // M,max_time_local_search
local_search_str          // l,local_search
"""

import os


def get_target(instance: str):
    with open("instances/best_scores_wvcp.txt", "r", encoding="UTF8") as file:
        for line in file.readlines():
            inst, score, optim = line[:-1].split(" ")
            if inst != instance:
                continue
            if optim == "*":
                return score
            return 0
    print(f"instance {instance} not found in instances/best_scores_wvcp.txt")


# instance  # i,instance
with open("all_instances.txt", "r", encoding="UTF8") as file:
    instances = [line[:-1] for line in file.readlines()]

method = "local_search"  # m,method

time_limit = 3600  # t,time_limit
# rand_seed  # r,rand_seed
rand_seeds = list(range(20))

target = 0  # T,target

nb_max_iterations = 9000000000000000000  # n,nb_max_iterations
nb_iter_local_search = 9000000000000000000  # N,nb_iter_local_search
max_time_local_search = time_limit  # M,max_time_local_search

initializations = [
    # "random",
    # "constrained",
    "deterministic"
]  # I,initialization

local_searchs = [
    # "hill_climbing",
    # "tabu_weight",
    # "tabu_col",
    # "afisa",
    # "afisa_original",
    # "redls",
    "ilsts",
]  # l,local_search

output_directory = "output_ls"

os.mkdir(f"{output_directory}/")
for local_search in local_searchs:
    for initialization in initializations:
        os.mkdir(f"{output_directory}/{local_search}")

with open("to_eval_ls", "w", encoding="UTF8") as file:
    for local_search in local_searchs:
        for initialization in initializations:
            for instance in instances:
                target = get_target(instance)
                for rand_seed in rand_seeds:
                    file.write(
                        f"./gc_wvcp "
                        f" -i {instance}"
                        f" -m {method}"
                        f" -t {time_limit}"
                        f" -r {rand_seed}"
                        f" -T {target}"
                        f" -I {initialization}"
                        f" -N {nb_iter_local_search}"
                        f" -M {max_time_local_search}"
                        f" -l {local_search}"
                        f" -o ../{output_directory}/{local_search}/{instance}_{rand_seed}.csv"
                        "\n"
                    )
