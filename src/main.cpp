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
 * @return std::shared_ptr<Method>
 */
std::shared_ptr<Method> parse(int argc, const char **argv);

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

std::shared_ptr<Method> parse(int argc, const char **argv) {
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
                // "p42"
                // "queen10_10"
                // "DSJC125.1gb"
                // "zeroin.i.3"
                // "DSJC500.5"
                "DSJC250.5"
                // "DSJC500.9"
                // "wap01a"
                // "C2000.9"
                // "DSJC250.5"
                // "r1000.5"
                //
                ));

        options.allow_unrecognised_options().add_options()(
            "m,method",
            "method (mcts, local_search)",
            cxxopts::value<std::string>()->default_value(
                //
                // "mcts"
                "local_search"
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

        // const std::string time_limit{"3600"};
        const std::string time_limit{"18000"};
        options.allow_unrecognised_options().add_options()(
            "t,time_limit",
            "maximum execution time in seconds",
            cxxopts::value<int>()->default_value(time_limit));

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
                "constrained"
                // "deterministic"
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
                time_limit
                // "-1"
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
                // "none"
                // "hill_climbing"
                // "tabu_weight"
                // "tabu_col"
                // "afisa"
                // "afisa_original"
                // "redls"
                "ilsts"
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
            cxxopts::value<int>()->default_value("4"));

        options.allow_unrecognised_options().add_options()(
            "P,P_time",
            "P to calculate the time of RL : O+P*nb_vertices seconds",
            cxxopts::value<double>()->default_value("0.01"));

        options.allow_unrecognised_options().add_options()(
            "o,ouput_to_file",
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

        // init rand seed
        const int rand_seed{result["rand_seed"].as<int>()};
        rd::generator.seed(rand_seed);

        const std::string problem{result["problem"].as<std::string>()};
        if (problem != "wvcp" and problem != "gcp") {
            fmt::print(stderr,
                       "unknown problem {}\n"
                       "select :\n"
                       "\twvcp (Weighted Vertex Coloring Problem)\n"
                       "\tgcp (Graph Coloring Problem)",
                       problem);
            exit(1);
        }

        // init graph
        Graph::init_graph(result["instance"].as<std::string>(), problem);

        const int max_time_local_search{
            result["max_time_local_search"].as<int>() == -1
                ? std::max(1,
                           static_cast<int>(static_cast<double>(Graph::g->nb_vertices) *
                                            result["P_time"].as<double>()) +
                               result["O_time"].as<int>())
                : result["max_time_local_search"].as<int>()};

        // init parameters
        Parameters::p =
            std::make_unique<Parameters>(result["problem"].as<std::string>(),
                                         result["method"].as<std::string>(),
                                         result["time_limit"].as<int>(),
                                         result["rand_seed"].as<int>(),
                                         result["target"].as<int>(),
                                         result["nb_max_iterations"].as<long>(),
                                         result["initialization"].as<std::string>(),
                                         result["nb_iter_local_search"].as<long>(),
                                         max_time_local_search,
                                         result["local_search"].as<std::string>(),
                                         result["simulation"].as<std::string>(),
                                         result["coeff_exploi_explo"].as<double>());

        // set output file if needed
        const std::string output_file{result["ouput_to_file"].as<std::string>()};
        if (output_file != "") {
            std::FILE *file = std::fopen((output_file + ".running").c_str(), "w");
            if (!file) {
                fmt::print(stderr, "error while trying to access {}\n", output_file);
                exit(1);
            }
            Parameters::p->output = file;
            Parameters::p->output_file = output_file;
        } else {
            Parameters::p->output = stdout;
        }

        const std::string method{result["method"].as<std::string>()};

        // create the method
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
