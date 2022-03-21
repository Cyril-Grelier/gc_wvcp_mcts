#pragma once

#include <string>
#include <vector>

// import fmt here to avoid importing it everywhere
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"
#include <fmt/printf.h>
#pragma GCC diagnostic pop

/**
 * @brief Represent the action of moving a vertex to a color that giving a score
 *
 */
struct Action {
    /** @brief vertex to color*/
    int vertex;
    /** @brief color to use*/
    int color;
    /** @brief next score*/
    int score;
};

/**
 * @brief Action are the same if they share the same values
 *
 * @param m1 first action
 * @param m2 second action
 * @return true m1 is equal to m2
 * @return false m1 is not equal to m2
 */
bool operator==(const Action &m1, const Action &m2);

/**
 * @brief Compare actions to sort it
 *
 * @param a first action
 * @param b second action
 * @return true a has a higher score than b
 * @return false a has a lower or equal score than b
 */
bool compare_actions(const Action &a, const Action &b);

/**
 * @brief Represent the action of moving a vertex to a color
 *
 */
struct Coloration {
    /** @brief vertex to color*/
    int vertex;
    /** @brief color to use*/
    int color;
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
