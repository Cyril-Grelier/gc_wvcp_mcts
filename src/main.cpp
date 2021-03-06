#include <csignal>
#include <fstream>
#include <memory>
#include <string>
#include <vector>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wnull-dereference"
#include "cxxopts.hpp"
#pragma GCC diagnostic pop

#include "methods/LocalSearch.h"
#include "methods/MCTS.h"
#include "representation/Graph.h"
#include "representation/Method.h"
#include "representation/Parameters.h"
#include "utils/random_generator.h"

/**
 * @brief Signal handler to let the algorithm to finish its last turn
 *
 * @param signum signal number
 */
void signal_handler(int signum);

/**
 * @brief parse the argument for the search
 *
 * @param argc : number of arguments given to main
 * @param argv : list of arguments
 * @return std::unique_ptr<Method>
 */
std::unique_ptr<Method> parse(int argc, const char **argv);

int main(int argc, const char *argv[]) {
    // see src/utils/parsing.cpp for default parameters
    // Get the method
    auto method(parse(argc, argv));

    // Set the signal handler to stop the search
    signal(SIGTERM, signal_handler);
    signal(SIGINT, signal_handler);

    // Start the search
    method->run();
    Parameters::p->end_search();
}

void signal_handler(int signum) {
    fmt::print(stderr, "\nInterrupt signal ({}) received.\n", signum);
    Parameters::p->time_stop = std::chrono::high_resolution_clock::now();
}

std::unique_ptr<Method> parse(int argc, const char **argv) {
    // defaults values for parameters are located in representation/Parameter.cpp

    // analyse command line options
    try {
        // init cxxopts
        cxxopts::Options options(argv[0], "Program to launch : mcts, local search\n");

        options.positional_help("[optional args]").show_positional_help();

        options.allow_unrecognised_options().add_options()("h,help", "Print usage");

        /****************************************************************************
         *
         *                      Set defaults values down here
         *
         ***************************************************************************/

        options.allow_unrecognised_options().add_options()(
            "p,problem",
            "problem (gcp, wvcp)",
            cxxopts::value<std::string>()->default_value(
                //
                "wvcp"
                // "gcp"
                //
                ));

        options.allow_unrecognised_options().add_options()(
            "i,instance",
            "name of the instance (located in instance/wvcp_reduced/)",
            cxxopts::value<std::string>()->default_value(
                //
                "p06"
                // "p42"
                // "queen10_10"
                // "DSJC125.1gb"
                // "zeroin.i.3"
                // "DSJC500.5"
                // "DSJC250.5"
                // "DSJC500.9"
                // "wap01a"
                // "C2000.9"
                // "DSJC250.5"
                // "r1000.5"
                // "le450_25a"
                // "zeroin.i.3"
                // "mulsol.i.5"
                //
                ));

        options.allow_unrecognised_options().add_options()(
            "m,method",
            "method (mcts, local_search)",
            cxxopts::value<std::string>()->default_value(
                //
                "mcts"
                // "local_search"
                //
                ));

        options.allow_unrecognised_options().add_options()(
            "r,rand_seed",
            "random seed",
            cxxopts::value<int>()->default_value(
                //
                // "1"
                std::to_string(time(nullptr))
                //
                ));

        options.allow_unrecognised_options().add_options()(
            "T,target",
            "if the target score is reach, the search is stopped",
            cxxopts::value<int>()->default_value("0"));

        options.allow_unrecognised_options().add_options()(
            "u,use_target",
            "for the mcts, if true, the mcts will prune the tree according to the target "
            "otherwise it will use the best found score during the search",
            cxxopts::value<bool>()->default_value(
                //
                "false"
                // "true"
                //
                ));

        options.allow_unrecognised_options().add_options()(
            "b,objective",
            "for mcts, does the algorithm stop when the target is reached or when "
            "optimality is proven? The algorithm can also stop when the time limit is "
            "reached (optimality, reached)",
            cxxopts::value<std::string>()->default_value(
                //
                "optimality"
                // "reached"
                //
                ));

        const std::string time_limit_default{"3600"};
        // const std::string time_limit_default{"18000"};
        options.allow_unrecognised_options().add_options()(
            "t,time_limit",
            "maximum execution time in seconds",
            cxxopts::value<int>()->default_value(time_limit_default));

        options.allow_unrecognised_options().add_options()(
            "n,nb_max_iterations",
            "number of iteration maximum for the mcts",
            cxxopts::value<long>()->default_value(
                std::to_string(std::numeric_limits<long>::max())));

        options.allow_unrecognised_options().add_options()(
            "I,initialization",
            "Initialization of the solutions (random, constrained, deterministic)",
            cxxopts::value<std::string>()->default_value(
                //
                // "random"
                // "constrained"
                "deterministic"
                //
                ));

        options.allow_unrecognised_options().add_options()(
            "N,nb_iter_local_search",
            "Number max of iteration for local search when call from another method or "
            "not, can be override by max_time_local_search or o and t time",
            cxxopts::value<long>()->default_value(
                //
                std::to_string(std::numeric_limits<long>::max())
                // "500"
                //
                ));

        options.allow_unrecognised_options().add_options()(
            "M,max_time_local_search",
            "Time limit in seconds for local search when call from another method or not "
            "can by override by nb_iter_local_search or o and t time",
            cxxopts::value<int>()->default_value(
                //
                // time_limit
                "-1"
                //
                ));

        options.allow_unrecognised_options().add_options()(
            "c,coeff_exploi_explo",
            "Coefficient exploration vs exploitation for MCTS",
            cxxopts::value<double>()->default_value("1"));

        options.allow_unrecognised_options().add_options()(
            "l,local_search",
            "Local search selected",
            cxxopts::value<std::string>()->default_value(
                //
                "none"
                // "hill_climbing"
                // "tabu_weight"
                // "tabu_col"
                // "afisa"
                // "afisa_original"
                // "redls"
                // "redls_freeze"
                // "ilsts"
                //
                ));

        options.allow_unrecognised_options().add_options()(
            "s,simulation",
            "Simulation for MCTS (greedy, local_search, depth, fit, depth_fit)",
            cxxopts::value<std::string>()->default_value(
                //
                "greedy"
                // "local_search"
                // "fit"
                // "depth"
                // "depth_fit"
                //
                ));

        options.allow_unrecognised_options().add_options()(
            "O,O_time",
            "O to calculate the time of RL : O+P*nb_vertices seconds",
            cxxopts::value<int>()->default_value("0"));

        options.allow_unrecognised_options().add_options()(
            "P,P_time",
            "P to calculate the time of RL : O+P*nb_vertices seconds",
            cxxopts::value<double>()->default_value("0.2"));

        options.allow_unrecognised_options().add_options()(
            "o,output_file",
            "output file, let empty if output to stdout",
            cxxopts::value<std::string>()->default_value(""));

        /****************************************************************************
         *
         *                      Set defaults values up here
         *
         ***************************************************************************/

        const auto result = options.parse(argc, const_cast<char **&>(argv));

        // help message
        if (result.count("help")) {
            // load instance names
            std::ifstream i_file("../instances/instance_list.txt");
            if (!i_file) {
                fmt::print(stderr,
                           "Unable to find : ../instances/instance_list.txt\n"
                           "Check if you imported the submodule instance, commands :\n"
                           "\tgit submodule init\n"
                           "\tgit submodule update\n");
                exit(1);
            }
            std::string tmp;
            std::vector<std::string> instance_names{};
            while (!i_file.eof()) {
                i_file >> tmp;
                instance_names.push_back(tmp);
            }
            i_file.close();
            // print help
            fmt::print(stdout,
                       "{}\nInstances :\n{}\n",
                       options.help(),
                       fmt::join(instance_names, " "));
            exit(0);
        }

        // get parameters
        const std::string problem = result["problem"].as<std::string>();
        if (problem != "wvcp" and problem != "gcp") {
            fmt::print(stderr,
                       "unknown problem {}\n"
                       "select :\n"
                       "\twvcp (Weighted Vertex Coloring Problem)\n"
                       "\tgcp (Graph Coloring Problem)",
                       problem);
            exit(1);
        }

        const std::string instance = result["instance"].as<std::string>();
        Graph::init_graph(instance, problem);

        const std::string method = result["method"].as<std::string>();

        const int rand_seed = result["rand_seed"].as<int>();
        rd::generator.seed(rand_seed);

        const int target = result["target"].as<int>();
        const bool use_target = result["use_target"].as<bool>();
        const std::string objective = result["objective"].as<std::string>();
        if (objective != "optimality" and objective != "reached") {
            fmt::print(stderr,
                       "unknown objective {}\n"
                       "select :\n"
                       "\toptimality (for MCTS, stop when tree completely explored)\n"
                       "\treached (for MCTS, stop when target reached)",
                       problem);
            exit(1);
        }

        const int time_limit = result["time_limit"].as<int>();
        const long nb_max_iterations = result["nb_max_iterations"].as<long>();
        const std::string initialization = result["initialization"].as<std::string>();
        const long nb_iter_local_search = result["nb_iter_local_search"].as<long>();

        int max_time_local_search = result["max_time_local_search"].as<int>();

        const double coeff_exploi_explo = result["coeff_exploi_explo"].as<double>();
        const std::string local_search = result["local_search"].as<std::string>();
        const std::string simulation = result["simulation"].as<std::string>();

        const int O_time = result["O_time"].as<int>();
        const double P_time = result["P_time"].as<double>();
        if (max_time_local_search == -1) {
            max_time_local_search = std::max(
                1,
                static_cast<int>(static_cast<double>(Graph::g->nb_vertices) * P_time) +
                    O_time);
        }

        const std::string output_file = result["output_file"].as<std::string>();

        // init parameters
        Parameters::p = std::make_unique<Parameters>(problem,
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
                                                     P_time,
                                                     output_file);

        // the method can't be created before the parameters
        if (method == "local_search") {
            return std::make_unique<LocalSearch>();
        }
        if (method == "mcts") {
            return std::make_unique<MCTS>();
        }

        fmt::print(stderr,
                   "error unknown method : {}\n"
                   "Possible method : mcts, local_search",
                   method);
        exit(1);

    } catch (const cxxopts::OptionException &e) {
        fmt::print(stderr, "error parsing options: {} \n", e.what());
        exit(1);
    }
}
