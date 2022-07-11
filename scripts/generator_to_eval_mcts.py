"""
Generate to_eval file which list all run to perform for MCTS

to split :
    split -l 1000 -d to_eval_mcts to_eval_mcts

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


# i,instance
with open("20_instances.txt", "r", encoding="UTF8") as file:
    instances = [line[:-1] for line in file.readlines()]


problem = "wvcp"  # gcp wvcp
method = "mcts"

rand_seeds = list(range(20))

target = 0
use_target = "false"
objective = "optimality"  # optimality reached
time_limit = 3600 * 5
nb_max_iterations = 9000000000000000000
initializations = [
    # "random",
    "constrained",
    # "deterministic",
]
nb_iter_local_search = 9000000000000000000
max_time_local_search = -1
coeff_exploi_explo = [
    "0",
    "0.25",
    "0.5",
    "0.75",
    "1",
    "1.25",
    "1.5",
    "1.75",
    "2",
]
local_searchs = [
    "none",
    # "tabu_col",
    # "hill_climbing",
    # "afisa_original",
    # "afisa",
    # "tabu_weight",
    # "redls",
    # "ilsts",
]
simulations = [
    "greedy",
    # "local_search",
    # "fit",
    # "depth",
    # "depth_fit",
]
O_time = 0
P_time = 0.02


output_directory = "output_mcts_constrained_coeff"

os.mkdir(f"{output_directory}/")
for initialization in initializations:
    for coeff in coeff_exploi_explo:
        for local_search in local_searchs:
            for simulation in simulations:
                # for P_time in P_times:
                os.mkdir(f"{output_directory}/{local_search}")

with open("to_eval_mcts", "w", encoding="UTF8") as file:
    for initialization in initializations:
        for coeff in coeff_exploi_explo:
            for local_search in local_searchs:
                for simulation in simulations:
                    for instance in instances:
                        target = 0  # get_target(instance)
                        for rand_seed in rand_seeds:
                            file.write(
                                f"./gc_wvcp "
                                f" --problem {problem}"
                                f" --instance {instance}"
                                f" --method {method}"
                                f" --rand_seed {rand_seed}"
                                f" --target {target}"
                                f" --use_target {use_target}"
                                f" --objective {objective}"
                                f" --time_limit {time_limit}"
                                f" --nb_max_iterations {nb_max_iterations}"
                                f" --initialization {initialization}"
                                f" --nb_iter_local_search {nb_iter_local_search}"
                                f" --max_time_local_search {max_time_local_search}"
                                f" --coeff_exploi_explo {coeff}"
                                f" --local_search {local_search}"
                                f" --simulation {simulation}"
                                f" --O_time {O_time}"
                                f" --P_time {P_time}"
                                f" --output_file ../{output_directory}/{local_search}_{P_time}/{instance}_{rand_seed}.csv"
                                "\n"
                            )
