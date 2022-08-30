"""
Generate to_eval file which list all run to perform for local search

to split :
    split -l 1000 -d to_eval_ls to_eval_ls

Parameters : 
    p,problem
    i,instance
    m,method
    r,rand_seed
    T,target
    u,use_target
    b,objective
    t,time_limit
    n,nb_max_iterations
    I,initialization
    N,nb_iter_local_search
    M,max_time_local_search
    c,coeff_exploi_explo
    l,local_search
    s,simulation
    O,O_time
    P,P_time
    o,output_file

"""

import os


def get_target(instance: str, problem: str):
    file = f"instances/best_scores_{problem}.txt"
    with open(file, "r", encoding="utf8") as file:
        for line in file.readlines():
            instance_, score, optimal = line[:-1].split(" ")
            if instance_ != instance:
                continue
            if optimal == "*":
                return int(score)
            return 0
    print(f"instance {instance} not found in instances/best_scores_wvcp.txt")


# i,instance
with open("instances/instance_list_wvcp.txt", "r", encoding="UTF8") as file:
    instances = [line[:-1] for line in file.readlines()]

problem = "wvcp"  # gcp wvcp
method = "local_search"

rand_seeds = list(range(20))

target = 0
use_target = "true"  # false true
objective = "reached"  # optimality reached
time_limit = 3600
nb_max_iterations = 9000000000000000000
initializations = [
    "random",
    "constrained",
    "deterministic",
]
nb_iter_local_search = 9000000000000000000
max_time_local_search = -1
local_searchs = [
    "none",
    # "tabu_col",
    # "hill_climbing",
    # "afisa_original",
    # "afisa",
    # "tabu_weight",
    # "redls",
    # "ilsts",
    # "redls_freeze",
]

O_time = 0
P_time = 0.02


output_directory = "greedy_only_all"

os.mkdir(f"{output_directory}/")
for local_search in local_searchs:
    for initialization in initializations:
        os.mkdir(f"{output_directory}/{initialization}")

with open("to_eval_ls", "w", encoding="UTF8") as file:
    for initialization in initializations:
        for local_search in local_searchs:
            for instance in instances:
                target = get_target(instance, problem)
                for rand_seed in rand_seeds:
                    file.write(
                        f"./gc_wvcp "
                        f" --problem {problem}"
                        f" --instance {instance}"
                        f" --method {method}"
                        f" --rand_seed {rand_seed}"
                        f" --target {target}"
                        f" --use_target true"
                        f" --objective reached"
                        f" --time_limit {time_limit}"
                        f" --nb_max_iterations {nb_max_iterations}"
                        f" --initialization {initialization}"
                        f" --nb_iter_local_search {nb_iter_local_search}"
                        f" --max_time_local_search {max_time_local_search}"
                        f" --coeff_exploi_explo 0"
                        f" --local_search {local_search}"
                        f" --simulation local_search"
                        f" --O_time {O_time}"
                        f" --P_time {P_time}"
                        f" --output_file ../{output_directory}/{initialization}/{instance}_{rand_seed}.csv"
                        "\n"
                    )
