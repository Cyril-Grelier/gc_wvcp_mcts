#pragma once

#include "../representation/Solution.h"

/** @brief Pointer to initialization function*/
typedef void (*init_ptr)(Solution &solution);

/**
 * @brief Greedy algorithm, that color vertices in the free_vertice order
 * and giving possibility to add a new color any time
 *
 * @param solution solution to use, the solution will be modified
 */
void greedy_random(Solution &solution);

/**
 * @brief Greedy algorithm, that color vertices in the free_vertice order
 * giving the first available color to the vertex
 *
 * @param solution solution to use, the solution will be modified
 */
void greedy_deterministic(Solution &solution);

/**
 * @brief Greedy algorithm, color the vertices in the order of the free_vertices list,
 * if no modification on the list, the list is ordered by weights and degrees of the
 * vertices from the _r.col file of the instance
 *
 * @param solution solution to use, the solution will be modified
 */
void greedy_constrained(Solution &solution);

/**
 * @brief Get the initialization
 *
 * @param initialization
 * @return init_ptr function greedy
 */
init_ptr get_initialization_fct(const Initialization &initialization);
