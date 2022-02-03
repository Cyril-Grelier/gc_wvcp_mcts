#include "Parameters.h"

#include <cstdio>

// to choose instance
const std::string Parameters::default_instance{"queen10_10"};

// to choose local_search or mcts
const std::string Parameters::default_method{
    //
    "mcts"
    // "local_search"
    //
};

// for local_search or mcts
const std::string Parameters::default_time_limit{"3600"};

// for local_search or mcts
const std::string Parameters::default_rand_seed{
    //
    // "1"
    std::to_string(time(nullptr))
    //
};

// for local_search and mcts (search stop when reached)
const std::string Parameters::default_target{"0"};

// for mcts
const std::string Parameters::default_nb_max_iterations{
    std::to_string(std::numeric_limits<long>::max())};

// for local_search and mcts
const std::string Parameters::default_initialization_str{
    //
    // "random"
    // "constrained"
    "deterministic"
    //
};

// for local_search (set to numeric_limits<long>::max) and mcts
// can be override by default_max_time_local_search
const std::string Parameters::default_nb_iter_local_search{
    //
    // std::to_string(std::numeric_limits<long>::max())
    "500"
    //
};

// for local_search (set to time limit) and mcts
// (see O and T in src/utils/parsing.cpp when set to -1)
// can be override by default_nb_iter_local_search
const std::string Parameters::default_max_time_local_search{
    //
    // Parameters::default_time_limit
    // "-1"
    "3600"
    //
};

// for local_search, mcts and memetic
const std::string Parameters::default_local_search_str{
    //
    "none"
    // "hill_climbing"
    // "tabu_weight"
    // "tabu_col"
    // "afisa"
    // "afisa_original"
    // "redls"
    // "ilsts"
    // "ilsts:redls:afisa"
    //
};

// for mcts
const std::string Parameters::default_coeff_exploi_explo{"1"};

// for mcts
const std::string Parameters::default_simulation_str{
    //
    "greedy"
    // "local_search"
    // "fit"
    // "depth"
    // "depth_fit"
    //
};

std::unique_ptr<Parameters> Parameters::p = nullptr;

void Parameters::init_parameters(std::unique_ptr<Parameters> parameters_) {
    Parameters::p = std::move(parameters_);
}

Parameters::Parameters(const std::string &method,
                       const int &time_limit_,
                       const int &rand_seed_,
                       const int &target_,
                       const long &nb_max_iterations_,
                       const std::string &initialization_str_,
                       const long &nb_iter_local_search_,
                       const int &max_time_local_search_,
                       const std::string &local_search_str_,
                       const std::string &simulation_str_,
                       const double &coeff_exploi_explo_)
    : time_start(std::chrono::high_resolution_clock::now()),
      time_limit(time_limit_),
      time_stop(time_start + std::chrono::seconds(time_limit_)),
      rand_seed(rand_seed_),
      target(target_),
      nb_max_iterations(nb_max_iterations_),
      initialization(string_to_initialization(initialization_str_)),
      nb_iter_local_search(nb_iter_local_search_),
      max_time_local_search(max_time_local_search_),
      local_search(string_to_vector_local_search(local_search_str_)),
      simulation(string_to_simulation(simulation_str_)),
      coeff_exploi_explo(coeff_exploi_explo_),
      initialization_str(initialization_str_),
      local_search_str(local_search_str_),
      simulation_str(simulation_str_) {
    if (method == "local_search") {
        header_csv = "time_limit,rand_seed,target,initialization,"
                     "nb_iter_local_search,max_time_local_search";
        line_csv = fmt::format("{},{},{},{},{},{}",
                               time_limit,
                               rand_seed,
                               target,
                               initialization_str,
                               nb_iter_local_search,
                               max_time_local_search);
    } else if (method == "mcts") {
        header_csv = "time_limit,rand_seed,target,nb_max_iterations,"
                     "nb_iter_local_search,max_time_local_search,initialization,"
                     "simulation,coeff_exploi_explo,local_search";
        line_csv = fmt::format("{},{},{},{},{},{},{},{},{},{}",
                               time_limit,
                               rand_seed,
                               target,
                               nb_max_iterations,
                               nb_iter_local_search,
                               max_time_local_search,
                               initialization_str,
                               simulation_str,
                               coeff_exploi_explo,
                               local_search_str);
    } else {
        fmt::print(stderr,
                   "Unknown method, please select : "
                   "local_search, mcts\n");
        exit(1);
    }
}

void Parameters::end_search() {
    if (output != stdout) {
        std::fflush(output);
        std::fclose(output);
        if (std::rename((output_file + ".running").c_str(), output_file.c_str()) != 0) {
            fmt::print(
                stderr, "error while changing name of output file {}\n", output_file);
            exit(1);
        }
    }
}

bool Parameters::time_limit_reached() {
    return not(std::chrono::duration_cast<std::chrono::seconds>(
                   time_stop - std::chrono::high_resolution_clock::now())
                   .count() >= 0);
}

bool Parameters::time_limit_reached(
    const std::chrono::high_resolution_clock::time_point &time) {
    return not(std::chrono::duration_cast<std::chrono::seconds>(
                   time - std::chrono::high_resolution_clock::now())
                   .count() >= 0);
}

int64_t
Parameters::elapsed_time(const std::chrono::high_resolution_clock::time_point &time) {
    return std::chrono::duration_cast<std::chrono::seconds>(time - time_start).count();
}
