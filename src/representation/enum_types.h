#pragma once

#include <string>
#include <vector>

// import fmt here to avoid importing it everywhere
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"
#include <fmt/printf.h>
#pragma GCC diagnostic pop

/** @brief Simulation for MCTS*/
enum class Simulation
{
    greedy,
    local_search,
    fit,
    depth,
    depth_fit
};

/** @brief Initialization for all methods*/
enum class Initialization
{
    random,
    constrained,
    deterministic
};

/** @brief Local_search for all methods*/
enum class Local_search
{
    none,
    hill_climbing,
    tabu_weight,
    tabu_col,
    afisa,
    afisa_original,
    redls,
    ilsts
};

/** @brief Perturbations for AFISA*/
enum class Perturbation
{
    no_perturbation,
    unlimited,
    no_tabu
};

// Convertion functions string to enum

/**
 * @brief Convert string to enum Simulation
 *
 * @param simulation Simulation name
 * @return Simulation enum
 */
Simulation string_to_simulation(const std::string &simulation);

/**
 * @brief Convert string to enum Initialization
 *
 * @param initialization Initialization name
 * @return Initialization enum
 */
Initialization string_to_initialization(const std::string &initialization);

/**
 * @brief Convert string to enum Local_search
 *
 * @param local_search local search name
 * @return Local_search enum
 */
Local_search string_to_local_search(const std::string &local_search);

/**
 * @brief Separate local_search in string (separated with ':')
 *
 * @param local_search string
 * @return std::vector<Local_search> enums
 */
std::vector<Local_search> string_to_vector_local_search(std::string local_search);
