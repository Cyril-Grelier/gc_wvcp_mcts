#include "hill_climbing.h"

#include "../utils/random_generator.h"
#include "../utils/utils.h"

void hill_climbing_one_move(Solution &solution, const bool verbose) {
    int64_t best_time{0};
    long turn{0};
    while (not Parameters::p->time_limit_reached() and
           solution.score_wvcp() != Parameters::p->target) {
        ++turn;
        std::vector<Action> best_actions;
        int best_evaluation = solution.score_wvcp();
        for (const auto &vertex : solution.free_vertices()) {
            for (const auto color : solution.non_empty_colors()) {
                if (color == solution.color(vertex) or
                    solution.conflicts_colors(color, vertex) != 0) {
                    continue;
                }
                const int test_score{solution.score_wvcp() +
                                     solution.delta_wvcp_score(vertex, color)};
                if (test_score < best_evaluation) {
                    best_actions.clear();
                    best_actions.emplace_back(Action{vertex, color, test_score});
                    best_evaluation = test_score;
                } else if (test_score == best_evaluation and not best_actions.empty()) {
                    best_actions.emplace_back(Action{vertex, color, test_score});
                }
            }
        }
        if (best_actions.empty()) {
            return;
        }
        const Action chosen_one{rd::choice(best_actions)};
        solution.delete_from_color(chosen_one.vertex);
        solution.add_to_color(chosen_one.vertex, chosen_one.color);
        if (verbose) {
            best_time =
                Parameters::p->elapsed_time(std::chrono::high_resolution_clock::now());
            print_result_ls(best_time, solution, turn);
        }
    }
}