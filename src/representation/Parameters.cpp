#include "Parameters.h"

#include <cstdio>
#include <fstream>

#include "../utils/utils.h"

std::unique_ptr<Parameters> Parameters::p = nullptr;

Parameters::Parameters(const std::string &problem_,
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
                       const std::string &output_file_)
    : problem(problem_),
      instance(instance_),
      method(method_),
      rand_seed(rand_seed_),
      target(target_),
      use_target(use_target_),
      objective(objective_),
      time_start(std::chrono::high_resolution_clock::now()),
      time_limit(time_limit_),
      time_stop(time_start + std::chrono::seconds(time_limit_)),
      nb_max_iterations(nb_max_iterations_),
      initialization(initialization_),
      nb_iter_local_search(nb_iter_local_search_),
      max_time_local_search(max_time_local_search_),
      coeff_exploi_explo(coeff_exploi_explo_),
      local_search(local_search_),
      simulation(simulation_),
      O_time(O_time_),
      P_time(P_time_),
      output_file(output_file_) {
    // set output file if needed
    if (output_file != "") {
        std::FILE *file = std::fopen((output_file + ".running").c_str(), "w");
        if (!file) {
            fmt::print(stderr, "error while trying to access {}\n", output_file);
            exit(1);
        }
        output = file;
    } else {
        output = stdout;
    }

    header_csv = "problem,"
                 "instance,"
                 "method,"
                 "rand_seed,"
                 "target,"
                 "use_target,"
                 "objective,"
                 "time_limit,"
                 "nb_max_iterations,"
                 "initialization,"
                 "nb_iter_local_search,"
                 "max_time_local_search,"
                 "coeff_exploi_explo,"
                 "local_search,"
                 "simulation,"
                 "O_time,"
                 "P_time";
    line_csv = fmt::format("{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{}",
                           problem,
                           instance,
                           method,
                           rand_seed,
                           target,
                           use_target,
                           objective,
                           time_limit,
                           nb_max_iterations,
                           initialization,
                           nb_iter_local_search,
                           max_time_local_search,
                           coeff_exploi_explo,
                           local_search,
                           simulation,
                           O_time,
                           P_time);
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
