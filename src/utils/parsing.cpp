#include "parsing.h"

#include <fstream>
#include <vector>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wnull-dereference"
#include "cxxopts.hpp"
#pragma GCC diagnostic pop

#include "../methods/LocalSearch.h"
#include "../methods/MCTS.h"
#include "../representation/Parameters.h"
#include "random_generator.h"

std::shared_ptr<Method> parse(int argc, const char **argv) {
    // defaults values for parameters are located in representation/Parameter.cpp

    // analyse command line options
    try {
        // init cxxopts
        cxxopts::Options options(argv[0], "Program to launch : mcts, local search\n");

        options.positional_help("[optional args]").show_positional_help();

        options.allow_unrecognised_options().add_options()("h,help", "Print usage");

        options.allow_unrecognised_options().add_options()(
            "i,instance",
            "name of the instance (located in instance/wvcp_reduced/)",
            cxxopts::value<std::string>()->default_value(Parameters::default_instance));

        options.allow_unrecognised_options().add_options()(
            "m,method",
            "method (mcts, local_search)",
            cxxopts::value<std::string>()->default_value(Parameters::default_method));

        options.allow_unrecognised_options().add_options()(
            "p,problem",
            "problem (gcp, wvcp)",
            cxxopts::value<std::string>()->default_value("gcp"));

        options.allow_unrecognised_options().add_options()(
            "t,time_limit",
            "maximum execution time in seconds",
            cxxopts::value<int>()->default_value(Parameters::default_time_limit));

        options.allow_unrecognised_options().add_options()(
            "r,rand_seed",
            "random seed",
            cxxopts::value<int>()->default_value(Parameters::default_rand_seed));

        options.allow_unrecognised_options().add_options()(
            "T,target",
            "if the target score is reach, the search is stopped",
            cxxopts::value<int>()->default_value(Parameters::default_target));

        options.allow_unrecognised_options().add_options()(
            "n,nb_max_iterations",
            "number of iteration maximum of the mcts",
            cxxopts::value<long>()->default_value(Parameters::default_nb_max_iterations));

        options.allow_unrecognised_options().add_options()(
            "I,initialization",
            "Initialization of the solutions",
            cxxopts::value<std::string>()->default_value(
                Parameters::default_initialization_str));

        options.allow_unrecognised_options().add_options()(
            "N,nb_iter_local_search",
            "Number max of iteration for local search when call from another method or "
            "not",
            cxxopts::value<long>()->default_value(
                Parameters::default_nb_iter_local_search));

        options.allow_unrecognised_options().add_options()(
            "M,max_time_local_search",
            "Time limit in seconds for local search when call from another method or not",
            cxxopts::value<int>()->default_value(
                Parameters::default_max_time_local_search));

        options.allow_unrecognised_options().add_options()(
            "c,coeff_exploi_explo",
            "Coefficient exploration vs exploitation for MCTS",
            cxxopts::value<double>()->default_value(
                Parameters::default_coeff_exploi_explo));

        options.allow_unrecognised_options().add_options()(
            "l,local_search",
            "Local search selected",
            cxxopts::value<std::string>()->default_value(
                Parameters::default_local_search_str));

        options.allow_unrecognised_options().add_options()(
            "s,simulation",
            "Simulation for MCTS",
            cxxopts::value<std::string>()->default_value(
                Parameters::default_simulation_str));

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
                       "select : wvcp, gcp",
                       problem);
            exit(1);
        }

        // init graph
        Graph::init_graph(
            load_graph(result["instance"].as<std::string>(), problem == "wvcp"));

        const int max_time_local_search{
            result["max_time_local_search"].as<int>() == -1
                ? static_cast<int>(static_cast<double>(Graph::g->nb_vertices) *
                                   result["P_time"].as<double>()) +
                      result["O_time"].as<int>()
                : result["max_time_local_search"].as<int>()};

        // init parameters
        Parameters::init_parameters(
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
                                         result["coeff_exploi_explo"].as<double>()));

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

const std::unique_ptr<const Graph> load_graph(const std::string &instance_name,
                                              const bool wvcp_problem) {
    // load the edges and vertices of the graph
    std::ifstream file;
    if (wvcp_problem) {
        file.open("../instances/wvcp_reduced/" + instance_name + ".col");
    } else {
        file.open("../instances/wvcp_original/" + instance_name + ".col");
    }
    if (!file) {
        fmt::print(stderr,
                   "Didn't find {} in ../instances/wvcp_reduced/ or wvcp_original (if "
                   "problem == gcp)\n"
                   "Did you run \n\n"
                   "git submodule init\n"
                   "git submodule update\n\n"
                   "before executing the program ?(import instances)\n"
                   "Otherwise check that you are in the build "
                   "directory before executing the program\n",
                   instance_name);
        exit(1);
    }
    int nb_vertices{0}, nb_edges{0}, n1{0}, n2{0};
    std::vector<std::pair<int, int>> edges_list;
    std::string first;
    file >> first;
    while (!file.eof()) {
        if (first == "e") {
            file >> n1 >> n2;
            edges_list.emplace_back(--n1, --n2);
        } else if (first == "p") {
            file >> first >> nb_vertices >> nb_edges;
            edges_list.reserve(nb_edges);
        } else {
            getline(file, first);
        }
        file >> first;
    }
    file.close();

    std::vector<int> weights(nb_vertices, 1);

    if (wvcp_problem) {
        // load the weights of the vertices
        std::ifstream w_file("../instances/wvcp_reduced/" + instance_name + ".col.w");
        if (!w_file) {
            fmt::print(stderr,
                       "Didn't find weights for {} in ../instances/wvcp_reduced/\n",
                       instance_name);
            exit(1);
        }
        size_t i(0);
        while (!w_file.eof()) {
            w_file >> weights[i];
            ++i;
        }
        w_file.close();
    }

    std::vector<std::vector<bool>> adjacency_matrix(
        nb_vertices, std::vector<bool>(nb_vertices, false));
    std::vector<std::vector<int>> neighborhood(nb_vertices, std::vector<int>(0));
    std::vector<int> degrees(nb_vertices, 0);
    // Init adjacency matrix and neighborhood of the vertices
    for (auto p : edges_list) {
        if (not adjacency_matrix[p.first][p.second]) {
            adjacency_matrix[p.first][p.second] = true;
            adjacency_matrix[p.second][p.first] = true;
            neighborhood[p.first].push_back(p.second);
            neighborhood[p.second].push_back(p.first);
            ++nb_edges;
        }
    }
    // Init degrees_ of the vertices
    for (int vertex{0}; vertex < nb_vertices; ++vertex) {
        degrees[vertex] = static_cast<int>(neighborhood[vertex].size());
    }
    // Uncomment to check if the vertices are well sorted
    // for(int vertex(0); vertex < nb_vertices-1; ++vertex){
    //     if(weights[vertex] < weights[vertex + 1] or (
    //         weights[vertex] == weights[vertex + 1] and
    //         degrees[vertex] < degrees[vertex + 1]
    //     )){
    //         fmt::print(stderr,"error v{}w{}d{} before v{}w{}d{}\nVertices must be
    //         sorted\n", vertex, weights[vertex], degrees[vertex],vertex,
    //         weights[vertex], degrees[vertex]);
    //     }
    // }
    return std::make_unique<Graph>(instance_name,
                                   nb_vertices,
                                   nb_edges,
                                   edges_list,
                                   adjacency_matrix,
                                   neighborhood,
                                   degrees,
                                   weights);
}
