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

    static std::unique_ptr<Parameters> p; /** @brief The parameters of the search*/

    const std::string problem;
    const std::string instance;
    const std::string method;
    const int rand_seed;
    const int target; /** @brief Best known score*/
    const bool use_target;
    const std::string objective;
    const std::chrono::high_resolution_clock::time_point time_start;
    const int time_limit;
    std::chrono::high_resolution_clock::time_point
        time_stop; /** @brief time limit for the algorithm, can be set to now to stop*/
    const long nb_max_iterations; /** @brief Number of iteration maximum for the MCTS*/
    const std::string initialization;
    const long nb_iter_local_search;
    const int max_time_local_search;
    const double coeff_exploi_explo;
    const std::string local_search;
    const std::string simulation;
    const int O_time;
    const double P_time;
    const std::string output_file; /** @brief Output file name if not on console*/
    std::FILE *output{nullptr};    /** @brief Output, stdout if console*/
    std::string header_csv{};
    std::string line_csv{};

    /**
     * @brief Set parameters for the search
     *
     * @param parameters_ the parameters
     */
    static void init_parameters(std::unique_ptr<Parameters> parameters_);

    /**
     * @brief Construct Parameters
     */
    explicit Parameters(const std::string &problem_,
                        const std::string &instance_,
                        const std::string &method_,
                        int rand_seed_,
                        int target_,
                        bool use_target_,
                        const std::string &objective_,
                        int time_limit_,
                        long nb_max_iterations_,
                        const std::string &initialization_,
                        long nb_iter_local_search_,
                        int max_time_local_search_,
                        double coeff_exploi_explo_,
                        const std::string &local_search_,
                        const std::string &simulation_,
                        int O_time_,
                        double P_time_,
                        const std::string &output_file_);

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
