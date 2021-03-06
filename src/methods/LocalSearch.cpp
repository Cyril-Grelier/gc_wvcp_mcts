#include "LocalSearch.h"

#include "../utils/utils.h"
#include "afisa.h"
#include "afisa_original.h"
#include "hill_climbing.h"
#include "ilsts.h"
#include "redls.h"
#include "redls_freeze.h"
#include "tabu_col.h"
#include "tabu_weight.h"

LocalSearch::LocalSearch()
    : _best_solution(),
      _init_function(get_initialization_fct(Parameters::p->initialization)),
      _local_search_function(get_local_search_fct(Parameters::p->local_search)) {
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
    return fmt::format(
        "date,{},turn,time,{}\n", Parameters::p->header_csv, Solution::header_csv);
}

[[nodiscard]] const std::string LocalSearch::line_csv() const {
    return fmt::format(
        "{},{},{},{},{}\n",
        get_date_str(),
        Parameters::p->line_csv,
        0, // turn,
        Parameters::p->elapsed_time(std::chrono::high_resolution_clock::now()),
        _best_solution.line_csv());
}

local_search_ptr get_local_search_fct(const std::string &local_search) {
    if (local_search == "none")
        return nullptr;
    if (local_search == "hill_climbing")
        return hill_climbing_one_move;
    if (local_search == "tabu_col")
        return tabu_col;
    if (local_search == "tabu_weight")
        return tabu_weight;
    if (local_search == "afisa")
        return afisa;
    if (local_search == "afisa_original")
        return afisa_original;
    if (local_search == "redls")
        return redls;
    if (local_search == "redls_freeze")
        return redls_freeze;
    if (local_search == "ilsts")
        return ilsts;

    fmt::print(stderr,
               "Unknown local_search, please select : "
               "none, hill_climbing, tabu_col, tabu_weight, "
               "afisa, afisa_original, redls, redls_freeze, ilsts\n");
    exit(1);
}
