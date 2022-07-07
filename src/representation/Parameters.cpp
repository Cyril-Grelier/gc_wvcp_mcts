#include "Parameters.h"

#include <cstdio>

#include "../utils/utils.h"

std::unique_ptr<Parameters> Parameters::p = nullptr;

Parameters::Parameters(const std::string &problem,
                       const std::string &method,
                       const int &time_limit_,
                       const int &rand_seed_,
                       const int &target_,
                       const long &nb_max_iterations_,
                       const std::string &initialization_,
                       const long &nb_iter_local_search_,
                       const int &max_time_local_search_,
                       const std::string &local_search_,
                       const std::string &simulation_,
                       const double &coeff_exploi_explo_)
    : time_start(std::chrono::high_resolution_clock::now()),
      time_limit(time_limit_),
      time_stop(time_start + std::chrono::seconds(time_limit_)),
      rand_seed(rand_seed_),
      target(target_),
      nb_max_iterations(nb_max_iterations_),
      initialization(initialization_),
      nb_iter_local_search(nb_iter_local_search_),
      max_time_local_search(max_time_local_search_),
      local_search(local_search_),
      simulation(simulation_),
      coeff_exploi_explo(coeff_exploi_explo_) {
    if (method == "local_search") {
        header_csv = "problem,time_limit,rand_seed,"
                     "target,initialization,nb_iter_local_search,"
                     "max_time_local_search";
        line_csv = fmt::format("{},{},{},{},{},{},{}",
                               problem,
                               time_limit,
                               rand_seed,
                               target,
                               initialization_,
                               nb_iter_local_search,
                               max_time_local_search);
    } else if (method == "mcts") {
        header_csv = "problem,time_limit,rand_seed,"
                     "target,nb_max_iterations,nb_iter_local_search,"
                     "max_time_local_search,initialization,simulation,"
                     "coeff_exploi_explo,local_search";
        line_csv = fmt::format("{},{},{},{},{},{},{},{},{},{},{}",
                               problem,
                               time_limit,
                               rand_seed,
                               target,
                               nb_max_iterations,
                               nb_iter_local_search,
                               max_time_local_search,
                               initialization_,
                               simulation_,
                               coeff_exploi_explo,
                               local_search_);
    } else {
        fmt::print(stderr,
                   "Unknown method, please select : "
                   "local_search, mcts\n");
        exit(1);
    }
}

void Parameters::end_search() const {
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

bool Parameters::time_limit_reached() const {
    return not(std::chrono::duration_cast<std::chrono::seconds>(
                   time_stop - std::chrono::high_resolution_clock::now())
                   .count() >= 0);
}

bool Parameters::time_limit_reached_sub_method(
    const std::chrono::high_resolution_clock::time_point &time) const {
    return not(std::chrono::duration_cast<std::chrono::seconds>(
                   time - std::chrono::high_resolution_clock::now())
                   .count() >= 0) or
           not(std::chrono::duration_cast<std::chrono::seconds>(
                   time_stop - std::chrono::high_resolution_clock::now())
                   .count() >= 0);
}

int64_t Parameters::elapsed_time(
    const std::chrono::high_resolution_clock::time_point &time) const {
    return std::chrono::duration_cast<std::chrono::seconds>(time - time_start).count();
}
