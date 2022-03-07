#include "afisa_original.h"

#include "../utils/random_generator.h"
#include "../utils/utils.h"

void afisa_original(Solution &best_solution, const bool verbose) {
    // best_solution stay legal during the search, its updated when a
    // new best score with no penalty is found

    // for MCTS
    const int first_freeze_vertex{best_solution.get_rank_placed_vertices()};
    if ((Graph::g->nb_vertices - first_freeze_vertex) < (Graph::g->nb_vertices * 0.20)) {
        return;
    }

    auto max_time{std::chrono::high_resolution_clock::now() +
                  std::chrono::seconds(Parameters::p->max_time_local_search)};

    int64_t best_time{0};
    std::uniform_int_distribution<int> distribution(0, 100);
    // best_afisa_sol can have conflict between vertices
    Solution best_afisa_sol = best_solution;
    int penalty_coeff{1};
    int no_improvement{0};
    long turn_afisa{0};
    const long small_perturbation{static_cast<long>(0.05 * Graph::g->nb_vertices)};
    const long large_perturbation{static_cast<long>(0.5 * Graph::g->nb_vertices)};
    long perturbation{small_perturbation};
    const long nb_turn_tabu{Graph::g->nb_vertices * 10};
    // main loop of the program
    while (not Parameters::p->time_limit_reached_sub_method(max_time) and
           turn_afisa < Parameters::p->nb_iter_local_search and
           best_solution.score() != Parameters::p->target) {
        ++turn_afisa;

        Solution solution = best_afisa_sol;
        // tabu phase
        afisa_original_tabu(solution,
                            best_solution,
                            best_afisa_sol,
                            penalty_coeff,
                            nb_turn_tabu,
                            Perturbation::no_perturbation,
                            max_time);

        // if new best score found
        if (best_afisa_sol.score() < best_solution.score() and
            best_afisa_sol.nb_conflicts() == 0) {
            best_solution = best_afisa_sol;
            no_improvement = 0;
            perturbation = small_perturbation;
            if (verbose) {
                best_time = Parameters::p->elapsed_time(
                    std::chrono::high_resolution_clock::now());
                fmt::print(Parameters::p->output,
                           "{},{},{},{},{},{},{}\n",
                           get_date_str(),
                           Parameters::p->local_search_str,
                           Graph::g->name,
                           Parameters::p->line_csv,
                           turn_afisa,
                           best_time,
                           best_solution.line_csv());
            }
        } else {
            no_improvement++;
            if (no_improvement == 50) {
                perturbation = large_perturbation;
            }
        }

        // adaptive adjusment
        if (best_afisa_sol.nb_conflicts() != 0) {
            ++penalty_coeff;
        } else {
            --penalty_coeff;
            if (penalty_coeff <= 0) {
                penalty_coeff = 1;
            }
        }

        // perturbation phase
        afisa_original_tabu(solution,
                            best_solution,
                            best_afisa_sol,
                            penalty_coeff,
                            perturbation,
                            (distribution(rd::generator) < 50 ? Perturbation::no_tabu
                                                              : Perturbation::unlimited),
                            max_time);
    }
    if (verbose) {
        fmt::print(Parameters::p->output,
                   "{},{},{},{},{},{},{}\n",
                   get_date_str(),
                   Parameters::p->local_search_str,
                   Graph::g->name,
                   Parameters::p->line_csv,
                   turn_afisa,
                   best_time,
                   best_solution.line_csv());
    }
}

void afisa_original_tabu(Solution &solution,
                         const Solution &best_solution,
                         Solution &best_afisa_sol,
                         const int &penalty_coeff,
                         const long &turns,
                         const Perturbation &perturbation,
                         const std::chrono::high_resolution_clock::time_point &max_time) {
    const int nb_max_colors{static_cast<int>(std::max(solution.nb_colors(), 14) * 1.10)};
    std::vector<std::vector<long>> tabu_matrix(Graph::g->nb_vertices,
                                               std::vector<long>(nb_max_colors, 0));
    std::uniform_int_distribution<int> distribution(0, 10);
    // tabu search loop
    long turn_tabu{0};
    while (not Parameters::p->time_limit_reached_sub_method(max_time) and
           turn_tabu < turns) {
        turn_tabu++;
        std::vector<Action> best_actions;
        int best_evaluation{std::numeric_limits<int>::max()};
        for (const auto &vertex : solution.free_vertices()) {
            for (int color{0}; color < nb_max_colors; ++color) {
                if (color == solution.color(vertex) or color >= solution.nb_colors()) {
                    continue;
                }
                // penalty is
                // - penalty from current position if the vertex move to a new color
                // new penalty - penalty from current position otherwise
                const int delta_penalty{
                    solution.nb_colors() == color
                        ? -solution.conflicts_colors(solution.color(vertex), vertex)
                        : solution.conflicts_colors(color, vertex) -
                              solution.conflicts_colors(solution.color(vertex), vertex)};
                const int test_score{
                    solution.score() + solution.get_delta_score(vertex, color) +
                    penalty_coeff * (delta_penalty + solution.nb_conflicts())};
                // save best moves
                if ((test_score < best_evaluation and
                     tabu_matrix[vertex][color] <= turn_tabu) or
                    (test_score < best_solution.score() and
                     (solution.nb_conflicts() + delta_penalty == 0))) {
                    best_actions.clear();
                    best_actions.emplace_back(Action{vertex, color, test_score});
                    best_evaluation = test_score;
                } else if (test_score == best_evaluation and
                           (tabu_matrix[vertex][color] <= turn_tabu or
                            (test_score < best_solution.score() and
                             (solution.nb_conflicts() + delta_penalty == 0)))) {
                    best_actions.emplace_back(Action{vertex, color, test_score});
                }
            }
        }
        // check if a best move if found (may be empty depending on the size of the tabu
        // list)
        if (not best_actions.empty()) {
            const Action chosen_one{rd::get_random_value(best_actions)};
            const int old_color{solution.delete_vertex_from_color(chosen_one.vertex)};
            if (solution.is_color_empty(chosen_one.color)) {
                solution.add_vertex_to_color(chosen_one.vertex, solution.add_new_color());
            } else {
                solution.add_vertex_to_color(chosen_one.vertex, chosen_one.color);
            }
            // set tabu
            switch (perturbation) {
            case Perturbation::no_perturbation:
                tabu_matrix[chosen_one.vertex][old_color] =
                    static_cast<int>(turn_tabu) + distribution(rd::generator) +
                    static_cast<int>(solution.score() +
                                     penalty_coeff * solution.nb_conflicts() * 0.6);
                break;
            case Perturbation::unlimited:
                tabu_matrix[chosen_one.vertex][old_color] = turns + 1;
                break;
            case Perturbation::no_tabu:
                break;
            }

            if ((solution.score() + penalty_coeff * solution.nb_conflicts()) <
                (best_afisa_sol.score() +
                 penalty_coeff * best_afisa_sol.nb_conflicts())) {
                best_afisa_sol = solution;
            }

            if (solution.score() == Parameters::p->target) {
                return;
            }
        }
    }
}
