#include "tabu_weight.h"

#include "../utils/random_generator.h"
#include "../utils/utils.h"

void tabu_weight(Solution &best_solution, const bool verbose) {
    const int first_freeze_vertex{best_solution.get_rank_placed_vertices()};

    if ((Graph::g->nb_vertices - first_freeze_vertex) < (Graph::g->nb_vertices * 0.20)) {
        return;
    }

    auto max_time{std::chrono::high_resolution_clock::now() +
                  std::chrono::seconds(Parameters::p->max_time_local_search)};

    int64_t best_time{0};

    Solution solution = best_solution;
    solution.reset_tabu();
    long turn{0};
    while (not Parameters::p->time_limit_reached_sub_method(max_time) and
           turn < Parameters::p->nb_iter_local_search and
           best_solution.score() != Parameters::p->target) {
        ++turn;
        std::vector<Action> best_actions;
        int best_evaluation{std::numeric_limits<int>::max()};
        for (const auto &vertex : solution.free_vertices()) {
            for (int color{0}; color < solution.nb_colors() + 1; ++color) {
                if (color == solution.color(vertex) or
                    (color < solution.nb_colors() and
                     solution.conflicts_colors(color, vertex) != 0)) {
                    continue;
                }
                const int test_score{solution.score() +
                                     solution.get_delta_score(vertex, color)};
                if ((test_score < best_evaluation and solution.tabu[vertex] <= turn) or
                    (test_score < best_solution.score())) {
                    best_actions.clear();
                    best_actions.emplace_back(Action{vertex, color, test_score});
                    best_evaluation = test_score;
                } else if (test_score == best_evaluation and
                           (solution.tabu[vertex] <= turn or
                            test_score < best_solution.score())) {
                    best_actions.emplace_back(Action{vertex, color, test_score});
                }
            }
        }
        if (not best_actions.empty()) {
            const Action chosen_one{rd::get_random_value(best_actions)};
            solution.delete_vertex_from_color(chosen_one.vertex);
            solution.add_vertex_to_color(chosen_one.vertex, chosen_one.color);
            solution.tabu[chosen_one.vertex] = turn + solution.nb_non_empty_colors();
            if (solution.score() < best_solution.score()) {
                best_solution = solution;
                if (verbose) {
                    best_time = Parameters::p->elapsed_time(
                        std::chrono::high_resolution_clock::now());
                    fmt::print(Parameters::p->output,
                               "{},{},{},{},{},{},{}\n",
                               get_date_str(),
                               Parameters::p->local_search_str,
                               Graph::g->name,
                               Parameters::p->line_csv,
                               turn,
                               best_time,
                               best_solution.line_csv());
                }
            }
        }
    }
    if (verbose) {
        fmt::print(Parameters::p->output,
                   "{},{},{},{},{},{},{}\n",
                   get_date_str(),
                   Parameters::p->local_search_str,
                   Graph::g->name,
                   Parameters::p->line_csv,
                   turn,
                   best_time,
                   best_solution.line_csv());
    }
}
