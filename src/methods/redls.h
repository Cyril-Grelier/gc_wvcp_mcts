#pragma once

#include "../representation/ProxiSolutionRedLS.h"

/**
 * @brief Local search from redLS
 *
 * From :
 * Wang, Y., Cai, S., Pan, S., Li, X., Yin, M., 2020.
 * Reduction and Local Search for Weighted Graph Coloring Problem.
 * AAAI 34, 2433–2441.
 * https://doi.org/10.1609/aaai.v34i03.5624
 *
 * @param solution solution to use, the solution will be modified
 * @param verbose True if print csv line each time new best scores is found
 */
void redls(Solution &solution, const bool verbose = false);

/**
 * @brief Return candidate moves set 1 (RedLS)
 *
 *
 * @param solution solution
 * @param best_local_score best score found in the local search
 * @return std::vector<Action> List of candidate moves in set 1
 */
bool candidate_set_1(ProxiSolutionRedLS &solution,
                     const int best_local_score,
                     std::vector<bool> &tabu_list);

/**
 * @brief Return candidate moves set 2 (RedLS)
 *
 * @param solution solution to use, the solution will be modified
 * @param withconf boolean flag indicating if the tabu list is activated
 * @return std::vector<Action> List of candidate moves in set 2
 */
bool candidate_set_2(ProxiSolutionRedLS &solution,
                     const bool withconf,
                     std::vector<bool> &tabu_list);

/**
 * @brief Return candidate moves set 3 (RedLS)
 *
 * @param solution solution to use, the solution will be modified
 * @param best_local_score best score found in the local search
 * @return std::vector<Action> List of candidate moves in set 3
 */
bool candidate_set_3(ProxiSolutionRedLS &solution,
                     const int best_local_score,
                     std::vector<bool> &tabu_list);

/**
 * @brief Return list of moves selected by Rule 1 (RedLS)
 *
 * @param solution solution to use, the solution will be modified
 * @return std::vector<Action> List of moves to apply
 */
void selectionRule1(ProxiSolutionRedLS &solution);

/**
 * @brief Return list of moves selected by Rule 2 (RedLS)
 *
 * @param solution solution to use, the solution will be modified
 * @param best_local_score best score found in the local search
 * @return std::vector<Action> List of moves to apply
 */
void selectionRule2(ProxiSolutionRedLS &solution,
                    const int best_local_score,
                    std::vector<bool> &tabu_list);
