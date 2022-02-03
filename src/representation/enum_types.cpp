#include "enum_types.h"

#include "../utils/utils.h"

Simulation string_to_simulation(const std::string &simulation) {
    if (simulation == "greedy") {
        return Simulation::greedy;
    } else if (simulation == "local_search") {
        return Simulation::local_search;
    } else if (simulation == "depth") {
        return Simulation::depth;
    } else if (simulation == "fit") {
        return Simulation::fit;
    } else if (simulation == "depth_fit") {
        return Simulation::depth_fit;
    }
    fmt::print(stderr,
               "Unknown simulation, please select : "
               "greedy, local_search,"
               "depth, fit or depth_fit\n");
    exit(1);
}

Initialization string_to_initialization(const std::string &initialization) {
    if (initialization == "random") {
        return Initialization::random;
    } else if (initialization == "constrained") {
        return Initialization::constrained;
    } else if (initialization == "deterministic") {
        return Initialization::deterministic;
    }
    fmt::print(stderr,
               "Unknown initialization, please select : "
               "random, constrained, deterministic\n");
    exit(1);
}

Local_search string_to_local_search(const std::string &local_search) {
    if (local_search == "none") {
        return Local_search::none;
    } else if (local_search == "hill_climbing") {
        return Local_search::hill_climbing;
    } else if (local_search == "tabu_col") {
        return Local_search::tabu_col;
    } else if (local_search == "tabu_weight") {
        return Local_search::tabu_weight;
    } else if (local_search == "afisa") {
        return Local_search::afisa;
    } else if (local_search == "afisa_original") {
        return Local_search::afisa_original;
    } else if (local_search == "redls") {
        return Local_search::redls;
    } else if (local_search == "ilsts") {
        return Local_search::ilsts;
    }
    fmt::print(stderr,
               "Unknown local_search, please select : "
               "none, hill_climbing, tabu_col, tabu_weight, "
               "afisa, afisa_original, redls, ilsts\n");
    exit(1);
}

std::vector<Local_search> string_to_vector_local_search(std::string local_search) {
    std::vector<Local_search> local_searches;
    for (const auto &ls : split_string(local_search, ":")) {
        local_searches.emplace_back(string_to_local_search(ls));
    }
    return local_searches;
}
