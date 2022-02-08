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

    /** @brief The parameters of the search*/
    static std::unique_ptr<Parameters> p;

    // default values for parameters (edit in Parameters.cpp)
    /** @brief Default parameter for instance*/
    const static std::string default_instance; // i,instance
    /** @brief Default parameter for method*/
    const static std::string default_method; // m,method
    /** @brief Default parameter for time_limit*/
    const static std::string default_time_limit; // t,time_limit
    /** @brief Default parameter for rand_seed*/
    const static std::string default_rand_seed; // r,rand_seed
    /** @brief Default parameter for target*/
    const static std::string default_target; // T,target
    /** @brief Default parameter for nb_max_iterations*/
    const static std::string default_nb_max_iterations; // n,nb_max_iterations
    /** @brief Default parameter for initialization_str*/
    const static std::string default_initialization_str; // I,initialization
    /** @brief Default parameter for nb_iter_local_search*/
    const static std::string default_nb_iter_local_search; // N,nb_iter_local_search
    /** @brief Default parameter for max_time_local_search*/
    const static std::string default_max_time_local_search; // M,max_time_local_search
    /** @brief Default parameter for coeff_exploi_explo*/
    const static std::string default_coeff_exploi_explo; // c,coeff_exploi_explo
    /** @brief Default parameter for local_search_str*/
    const static std::string default_local_search_str; // l,local_search
    /** @brief Default parameter for simulation_str*/
    const static std::string default_simulation_str; // s,simulation

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

    /** @brief Initialization for printing*/
    const std::string initialization_str;
    /** @brief Local_search for printing*/
    const std::string local_search_str;
    /** @brief Simulation for printing*/
    const std::string simulation_str;

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
    void end_search();

    /**
     * @brief Return true if the time limit is reached
     *
     * @return true Time limit is reached
     * @return false The search continue
     */
    bool time_limit_reached();

    /**
     * @brief Return true if the time limit is reached according to the given time
     *
     * @return true Time limit is reached
     * @return false The search continue
     */
    bool time_limit_reached(const std::chrono::high_resolution_clock::time_point &time);

    /**
     * @brief Returns the number of seconds between the given time and the start of
     * the search
     *
     * @param time given time (std::chrono::high_resolution_clock::now())
     * @return int64_t elapsed time in seconds
     */
    int64_t elapsed_time(const std::chrono::high_resolution_clock::time_point &time);
};
