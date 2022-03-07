#pragma once

#include <chrono>
#include <string>
#include <vector>

#include "enum_types.h"

/**
 * @brief Representation of parameters
 *
 */
struct Parameters {

    /** @brief The parameters of the search, refer as Parameters::p*/
    static std::unique_ptr<Parameters> p;

    /** @brief Time of the beginning of the search*/
    const std::chrono::high_resolution_clock::time_point time_start;
    /** @brief Time limit for the algorithm*/
    const int time_limit;
    /** @brief time limit for the algorithm, can be set to now to stop*/
    std::chrono::high_resolution_clock::time_point time_stop;
    /** @brief Rand seed given in parameter*/
    const int rand_seed;
    /** @brief Target to reach before stopping the search*/
    const int target;

    /** @brief Number of iteration maximum for the MCTS*/
    const long nb_max_iterations;

    /** @brief Type of initialization of the solutions*/
    const Initialization initialization;

    /** @brief Number of iteration for the local search*/
    const long nb_iter_local_search;
    /** @brief Maximum time for a run of local search*/
    const int max_time_local_search;

    /** @brief Local search (any method)*/
    const std::vector<Local_search> local_search;

    // MCTS

    /** @brief Type of simulation (MCTS)*/
    Simulation simulation;
    /** @brief Coefficient exploration vs exploitation (MCTS)*/
    const double coeff_exploi_explo;

    /** @brief Header for csv representation*/
    std::string header_csv{};
    /** @brief Line of data for csv representation*/
    std::string line_csv{};

    /** @brief Local_search for printing*/
    const std::string local_search_str;

    /** @brief Output, stdout if console (set in utils/parsing.cpp*/
    std::FILE *output = stdout;
    /** @brief Output file name if not on console*/
    std::string output_file = "";

    /**
     * @brief Set parameters for the search
     *
     * @param parameters_ the parameters
     */
    static void init_parameters(std::unique_ptr<Parameters> parameters_);

    /**
     * @brief Construct Parameters
     */
    explicit Parameters(const std::string &problem,
                        const std::string &method,
                        const int &time_limit_,
                        const int &rand_seed_,
                        const int &target_,
                        const long &nb_max_iterations_,
                        const std::string &initialization_str_,
                        const long &nb_iter_local_search_,
                        const int &max_time_local_search_,
                        const std::string &local_search_str_,
                        const std::string &simulation_str_,
                        const double &coeff_exploi_explo_);

    /**
     * @brief Close output file if needed
     *
     */
    void end_search() const;

    /**
     * @brief Return true if the time limit is reached
     *
     * @return true Time limit is reached
     * @return false The search continue
     */
    bool time_limit_reached() const;

    /**
     * @brief Return true if the time limit is reached according to the given time
     *
     * @return true Time limit is reached
     * @return false The search continue
     */
    bool time_limit_reached_sub_method(
        const std::chrono::high_resolution_clock::time_point &time) const;

    /**
     * @brief Returns the number of seconds between the given time and the start of
     * the search
     *
     * @param time given time (std::chrono::high_resolution_clock::now())
     * @return int64_t elapsed time in seconds
     */
    int64_t
    elapsed_time(const std::chrono::high_resolution_clock::time_point &time) const;
};
