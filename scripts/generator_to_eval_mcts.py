"""
Generate to_eval file which list all run to perform for MCTS

to split :
    split -l 1000 -d to_eval_mcts to_eval_mcts

instance                  // i,instance
problem                   // p,problem
method                    // m,method
time_limit                // t,time_limit
rand_seed                 // r,rand_seed
target                    // T,target
nb_max_iterations         // n,nb_max_iterations
initialization_str        // I,initialization
nb_iter_local_search      // N,nb_iter_local_search
max_time_local_search     // M,max_time_local_search
coeff_exploi_explo        // c,coeff_exploi_explo
local_search_str          // l,local_search
simulation_str            // s,simulation
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

problem = "wvcp" # "wvcp" "gcp" # p, problem
method = "mcts"  # m,method
time_limit = 3600  # t,time_limit
# rand_seed  # r,rand_seed
rand_seeds = list(range(20))
target = 0  # T,target
nb_max_iterations = 9000000000000000000  # n,nb_max_iterations

nb_iter_local_search = 500  # N,nb_iter_local_search

max_time_local_search = 3600  # M,max_time_local_search

initializations = [
    # "random",
    # "constrained",
    "deterministic",
]  # I,initialization

coeff_exploi_explo = [
    # "0",
    # "0.25",
    # "0.5",
    # "0.75",
    "1",
    # "1.25",
    # "1.5",
    # "1.75",
    # "2",
]  # c,coeff_exploi_explo

local_searchs = [
    # "none",
    # "tabu_col",
    # "hill_climbing",
    # "afisa_original",
    "afisa",
    "tabu_weight",
    "redls",
    "ilsts",
]  # l,local_search

simulations = [
    # "greedy",
    "local_search",
    # "fit",
    # "depth",
    # "depth_fit",
]  # s,simulation

output_directory = "output_mcts_local_search_deterministic"

os.mkdir(f"{output_directory}/")
for initialization in initializations:
    for coeff in coeff_exploi_explo:
        for local_search in local_searchs:
            for simulation in simulations:
                os.mkdir(f"{output_directory}/{local_search}")
                # os.mkdir(f"{output_directory}/{initialization}{coeff}/{local_search}/{simulation}")

with open("to_eval_mcts_ls", "w", encoding="UTF8") as file:
    for initialization in initializations:
        for coeff in coeff_exploi_explo:
            for local_search in local_searchs:
                for simulation in simulations:
                    for instance in instances:
                        target = 0  # get_target(instance)
                        for rand_seed in rand_seeds:
                            file.write(
                                f"./gc_wvcp "
                                f" -p {problem}"
                                f" -i {instance}"
                                f" -m {method}"
                                f" -t {time_limit}"
                                f" -r {rand_seed}"
                                f" -T {target}"
                                f" -n {nb_max_iterations}"
                                f" -I {initialization}"
                                f" -N {nb_iter_local_search}"
                                f" -M {max_time_local_search}"
                                f" -c {coeff}"
                                f" -l {local_search}"
                                f" -s {simulation}"
                                f" -o ../{output_directory}/{local_search}/{instance}_{rand_seed}.csv"
                                "\n"
                            )
