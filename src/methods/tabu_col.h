#pragma once

#include "../representation/Solution.h"

/**
 * @brief TabuCol
 *
 * From :
 * Hertz, A., Werra, D., 1987.
 * Werra, D.: Using Tabu Search Techniques for Graph Coloring.
 * Computing 39, 345-351. Computing 39.
 * https://doi.org/10.1007/BF02239976
 *
 * This tabu search is adapted for our problem.
 * Normally, the tabu is initialised with a fixed number of color then the objective is to
 * reduce the number of conflict to 0.
 *
 * Here, the solution have no limit on the number of color.
 * So we will empty some colors and randomly add the unassigned vertices to other colors
 * to increase the number of conflicts on the edges. Then once a solution is found, reduce
 * the number of colors again and reduce the number of conflict again and again...
 *
 * @param solution solution to use, the solution will be modified
 * @param verbose True if print csv line each time new best scores is found
 */
void tabu_col(Solution &solution, const bool verbose = false);

/**
 * @brief Close colors and push unassigned vertices other colors and create conflicts
 *
 * @param nb_total_color Number of color the solution will have
 */
void reduce_nb_colors(Solution &solution, const int nb_total_color);
