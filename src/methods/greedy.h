#pragma once

#include "../representation/Solution.h"

/** @brief Pointer to initialization function*/
typedef void (*init_ptr)(Solution &solution);

/**
 * @brief Greedy algorithm, that color vertices starting with next vertex to color in the
 * MCTS tree (0 by default) and giving possibility to add a new color any time
 *
 * @param solution solution to use, the solution will be modified
 */
void greedy_random(Solution &solution);

/**
 * @brief Greedy algorithm, color the vertices starting with next vertex to color in the
 * MCTS tree (0 by default), add each vertex to a random color, create a new color only if
 * needed
 *
 * @param solution solution to use, the solution will be modified
 */
void greedy_constrained(Solution &solution);

/**
 * @brief Greedy algorithm, that color vertices starting with next vertex to color in the
 * MCTS tree (0 by default) giving the first available color to the vertex
 *
 * @param solution solution to use, the solution will be modified
 */
void greedy_deterministic(Solution &solution);

/**
 * @brief Worst greedy, one vertex per color
 *
 * @param solution solution to use, the solution will be modified
 */
void greedy_worst(Solution &solution);

/**
 * @brief Get the initialization
 *
 * @param initialization
 * @return init_ptr function greedy
 */
init_ptr get_initialization_fct(const std::string &initialization);
