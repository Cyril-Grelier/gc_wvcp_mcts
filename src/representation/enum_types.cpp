#include "enum_types.h"

#include "../utils/utils.h"

bool operator==(const Action &m1, const Action &m2) {
    return m1.color == m2.color and m1.vertex == m2.vertex and m1.score == m2.score;
}

bool compare_actions(const Action &a, const Action &b) {
    return (a.score > b.score) or (a.score == b.score and a.color > b.color);
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
