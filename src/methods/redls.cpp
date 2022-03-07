#include "redls.h"

#include "../utils/random_generator.h"
#include "../utils/utils.h"

void redls(Solution &best_solution, const bool verbose) {
    const int first_freeze_vertex{best_solution.get_rank_placed_vertices()};
    if ((Graph::g->nb_vertices - first_freeze_vertex) < (Graph::g->nb_vertices * 0.20)) {
        return;
    }

    auto max_time{std::chrono::high_resolution_clock::now() +
                  std::chrono::seconds(Parameters::p->max_time_local_search)};

    int64_t best_time{0};

    Solution solution = best_solution;
    solution.reset_tabu();
    solution.reset_edge_weights();
    long turn{0};
    while (not Parameters::p->time_limit_reached_sub_method(max_time) and
           turn < Parameters::p->nb_iter_local_search and
           best_solution.score() != Parameters::p->target) {
        ++turn;
        // Step 1 RedLS: ligne 4 - 11 algo 3
        if (solution.nb_conflicts() == 0) {
            bool continueCanSet2 = true;
            while (continueCanSet2) {
                const auto best_actions = get_moves_CanSet2(solution, false);
                if (not best_actions.empty()) {
                    const Action chosen_one{rd::get_random_value(best_actions)};
                    solution.delete_vertex_from_color(chosen_one.vertex);
                    solution.add_vertex_to_color(chosen_one.vertex, chosen_one.color);
                } else {
                    continueCanSet2 = false;
                }
            }
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
                if (solution.score() == Parameters::p->target) {
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
                    return;
                }
            }
            solution.reset_tabu();
            const auto best_actions = selectionRule1(solution);
            for (const auto &action : best_actions) {
                solution.delete_vertex_from_color(action.vertex);
                solution.add_vertex_to_color(action.vertex, action.color);
            }
        }
        // Step 2 RedLS: ligne 12 - 14 algo 3
        const auto best_moves_canSet1 =
            get_moves_CanSet1(solution, best_solution.score());
        if (not best_moves_canSet1.empty()) {
            Action best_move{best_moves_canSet1[0]};
            int best_score_move = 0;
            for (const auto &action : best_moves_canSet1) {
                if (action.score >= best_score_move) {
                    best_score_move = action.score;
                    best_move = action;
                }
            }
            solution.delete_vertex_from_color(best_move.vertex);
            solution.add_vertex_to_color(best_move.vertex, best_move.color);

            solution.tabu[best_move.vertex] = 1;
            for (const auto &neighbor : Graph::g->neighborhood[best_move.vertex]) {
                solution.tabu[neighbor] = 0;
            }
        } else {
            bool continueCanSet2 = true;
            while (continueCanSet2) {
                const auto best_moves_canSet2 = get_moves_CanSet2(solution, true);
                if (not best_moves_canSet2.empty()) {
                    const auto chosen_one{rd::get_random_value(best_moves_canSet2)};
                    solution.delete_vertex_from_color(chosen_one.vertex);
                    solution.add_vertex_to_color(chosen_one.vertex, chosen_one.color);
                    solution.tabu[chosen_one.vertex] = 1;
                } else {
                    continueCanSet2 = false;
                }
            }
            const auto best_moves_canSet3 =
                get_moves_CanSet3(solution, best_solution.score());
            if (not best_moves_canSet3.empty()) {
                const Action chosen_one{rd::get_random_value(best_moves_canSet3)};
                solution.delete_vertex_from_color(chosen_one.vertex);
                solution.add_vertex_to_color(chosen_one.vertex, chosen_one.color);
                solution.tabu[chosen_one.vertex] = 1;
            } else {
                // Increments edge weight
                solution.increment_edge_weights();
                if (not solution.conflict_edges().empty()) {
                    Action chosen_one = selectionRule2(solution, best_solution.score());
                    solution.delete_vertex_from_color(chosen_one.vertex);
                    solution.add_vertex_to_color(chosen_one.vertex, chosen_one.color);
                    solution.tabu[chosen_one.vertex] = 1;
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

std::vector<Action> get_moves_CanSet1(Solution &solution, const int best_local_score) {
    std::vector<Action> canSet1;
    int max_score_S = std::max(solution.score(), best_local_score);
    for (const auto &vertex : solution.free_vertices()) {
        if (solution.conflicts_colors(solution.color(vertex), vertex) > 0 and
            solution.tabu[vertex] == 0) {
            for (const auto &color : solution.non_empty_colors()) {
                if (color != solution.color(vertex)) {
                    const int delta_conflicts_score =
                        solution.conflicts_colors(color, vertex) -
                        solution.conflicts_colors(solution.color(vertex), vertex);
                    if (delta_conflicts_score < 0) {
                        const int delta_wvcp_score =
                            solution.get_delta_score(vertex, color);
                        const int test_score = solution.score() + delta_wvcp_score;
                        if (test_score < max_score_S) {
                            canSet1.emplace_back(
                                Action{vertex, color, -delta_conflicts_score});
                        }
                    }
                }
            }
        }
    }
    return canSet1;
}

std::vector<Action> get_moves_CanSet2(Solution &solution, const bool withconf) {
    std::vector<Action> canSet2;
    for (const auto &vertex : solution.free_vertices()) {
        if (withconf == false or solution.tabu[vertex] == 0) {
            for (const auto &color : solution.non_empty_colors()) {
                if (color != solution.color(vertex)) {
                    const int delta_conflicts =
                        solution.conflicts_colors(color, vertex) -
                        solution.conflicts_colors(solution.color(vertex), vertex);
                    if (delta_conflicts == 0) {
                        const int delta_score = solution.get_delta_score(vertex, color);
                        if (delta_score < 0) {
                            canSet2.emplace_back(
                                Action{vertex, color, solution.score() + delta_score});
                        }
                    }
                }
            }
        }
    }
    return canSet2;
}

std::vector<Action> get_moves_CanSet3(Solution &solution, const int best_local_score) {
    std::vector<Action> canSet3;
    for (const auto &vertex : solution.free_vertices()) {
        if (solution.tabu[vertex] == 0) {
            if (solution.conflicts_colors(solution.color(vertex), vertex) > 0) {
                const int test_score =
                    solution.score() + solution.get_delta_score(vertex, -1);
                if (test_score < best_local_score) {
                    canSet3.emplace_back(Action{vertex, -1, test_score});
                }
            }
        }
    }
    return canSet3;
}

std::vector<Action> selectionRule1(Solution &solution) {
    int cpt = 0;
    float best_global_score = -1.0;
    int best_j = -1;
    std::vector<int> bestMaxVi;
    for (const auto &color1 : solution.non_empty_colors()) {
        const std::vector<int> &Vi = solution.colors_vertices(color1);
        std::vector<int> maxVi;
        const int maxWeightVi = Graph::g->weights[Vi[0]];
        int second_max = 0;
        for (const auto &v : Vi) {
            if (Graph::g->weights[v] == maxWeightVi) {
                maxVi.emplace_back(v);
            } else if (Graph::g->weights[v] > second_max) {
                second_max = Graph::g->weights[v];
                break;
            }
        }
        const int delta_move_Vi = maxWeightVi - second_max;
        for (const auto &color2 : solution.non_empty_colors()) {
            if (color1 != color2) {
                int global_delta_conflict_score = 0;
                for (const auto &v : maxVi) {
                    global_delta_conflict_score += solution.conflicts_colors(color2, v);
                }
                const int max_weight_color2 = solution.max_weight(color2);
                const int global_delta_wvcp_score{maxWeightVi > max_weight_color2
                                                      ? delta_move_Vi - maxWeightVi +
                                                            max_weight_color2
                                                      : delta_move_Vi};
                float global_score{
                    global_delta_conflict_score != 0
                        ? static_cast<float>(global_delta_wvcp_score) /
                              static_cast<float>(global_delta_conflict_score)
                        : static_cast<float>(global_delta_wvcp_score) * 2};
                if (cpt == 0) {
                    best_global_score = global_score;
                    bestMaxVi = maxVi;
                    best_j = color2;
                } else if (global_score > best_global_score) {
                    best_global_score = global_score;
                    bestMaxVi = maxVi;
                    best_j = color2;
                }
                cpt += 1;
            }
        }
    }
    std::vector<Action> list_actions;
    for (const auto &v : bestMaxVi) {
        list_actions.emplace_back(Action{v, best_j, -1});
    }
    return list_actions;
}

Action selectionRule2(Solution &solution, const int best_local_score) {
    Action best_action;
    const auto &[v1, v2]{rd::get_random_value(solution.conflict_edges())};
    int cpt = 0;
    int best_score_conflicts{0};
    bool found = false;
    for (int i = 0; i < 2; ++i) {
        const int v{i == 0 ? v1 : v2};
        for (int color{0}; color < solution.nb_colors(); ++color) {
            if (color != solution.color(v)) {
                int delta_score_wvcp = solution.get_delta_score(v, color);
                if (delta_score_wvcp + solution.score() < best_local_score) {
                    int delta_conflicts_score =
                        solution.conflicts_colors(solution.color(v), v) -
                        solution.conflicts_colors(color, v);
                    found = true;
                    if (cpt == 0) {
                        best_score_conflicts = delta_conflicts_score;
                        best_action =
                            Action{v, solution.is_color_empty(color) ? -1 : color, -1};
                    } else if (delta_conflicts_score > best_score_conflicts) {
                        best_score_conflicts = delta_conflicts_score;
                        best_action =
                            Action{v, solution.is_color_empty(color) ? -1 : color, -1};
                    }
                    cpt += 1;
                }
            }
        }
    }
    if (found) {
        return best_action;
    }
    const int v = v1;
    std::vector<int> possible_colors;
    for (int color{0}; color < solution.nb_colors(); ++color) {
        if (color != solution.color(v)) {
            possible_colors.emplace_back(color);
        }
    }
    const int color{rd::get_random_value(possible_colors)};
    return Action{v, solution.is_color_empty(color) ? -1 : color, -1};
}
