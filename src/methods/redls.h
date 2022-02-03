#pragma once

#include "../representation/Solution.h"

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
std::vector<Action> get_moves_CanSet1(Solution &solution, const int best_local_score);

/**
 * @brief Return candidate moves set 2 (RedLS)
 *
 * @param solution solution to use, the solution will be modified
 * @param withconf boolean flag indicating if the tabu list is activated
 * @return std::vector<Action> List of candidate moves in set 2
 */
std::vector<Action> get_moves_CanSet2(Solution &solution, const bool withconf);

/**
 * @brief Return candidate moves set 3 (RedLS)
 *
 * @param solution solution to use, the solution will be modified
 * @param best_local_score best score found in the local search
 * @return std::vector<Action> List of candidate moves in set 3
 */
std::vector<Action> get_moves_CanSet3(Solution &solution, const int best_local_score);

/**
 * @brief Return list of moves selected by Rule 1 (RedLS)
 *
 * @param solution solution to use, the solution will be modified
 * @return std::vector<Action> List of moves to apply
 */
std::vector<Action> selectionRule1(Solution &solution);

/**
 * @brief Return list of moves selected by Rule 2 (RedLS)
 *
 * @param solution solution to use, the solution will be modified
 * @param best_local_score best score found in the local search
 * @return std::vector<Action> List of moves to apply
 */
Action selectionRule2(Solution &solution, const int best_local_score);
