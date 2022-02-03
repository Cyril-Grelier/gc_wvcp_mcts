#include "LocalSearch.h"

#include "../utils/utils.h"
#include "afisa.h"
#include "afisa_original.h"
#include "greedy.h"
#include "hill_climbing.h"
#include "ilsts.h"
#include "redls.h"
#include "tabu_col.h"
#include "tabu_weight.h"

LocalSearch::LocalSearch()
    : _best_solution(),
      _init_function(get_initialization_fct(Parameters::p->initialization)),
      _local_search_function(get_local_search_fct(Parameters::p->local_search[0])) {
    _init_function(_best_solution);
}

void LocalSearch::run() {
    fmt::print(Parameters::p->output, "{}", header_csv());
    fmt::print(Parameters::p->output, "{}", line_csv());
    if (_local_search_function) {
        _local_search_function(_best_solution, true);
    }
}

[[nodiscard]] const std::string LocalSearch::header_csv() const {
    return fmt::format("date,method,instance,{},turn,time,{}\n",
                       Parameters::p->header_csv,
                       Solution::header_csv);
}

[[nodiscard]] const std::string LocalSearch::line_csv() const {
    return fmt::format(
        "{},{},{},{},{},{},{}\n",
        get_date_str(),
        Parameters::p->local_search_str,
        Graph::g->name,
        Parameters::p->line_csv,
        0, // turn,
        Parameters::p->elapsed_time(std::chrono::high_resolution_clock::now()),
        _best_solution.line_csv());
}

init_ptr get_initialization_fct(const Initialization &initialization) {
    switch (initialization) {
    case Initialization::random:
        return greedy_random;
    case Initialization::constrained:
        return greedy_constrained;
    case Initialization::deterministic:
        return greedy_deterministic;
    default:
        return nullptr;
    }
}

local_search_ptr get_local_search_fct(const Local_search &local_search) {
    switch (local_search) {
    case Local_search::none:
        return nullptr;
    case Local_search::hill_climbing:
        return hill_climbing_one_move;
    case Local_search::tabu_col:
        return tabu_col;
    case Local_search::tabu_weight:
        return tabu_weight;
    case Local_search::afisa:
        return afisa;
    case Local_search::afisa_original:
        return afisa_original;
    case Local_search::redls:
        return redls;
    case Local_search::ilsts:
        return ilsts;
    default:
        return nullptr;
    }
}
