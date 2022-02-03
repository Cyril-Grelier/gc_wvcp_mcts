#include "ilsts.h"

#include <algorithm>
#include <assert.h>

#include "../utils/random_generator.h"
#include "../utils/utils.h"

void ilsts(Solution &best_solution, const bool verbose) {
    const int first_freeze_vertex{best_solution.get_rank_placed_vertices()};

    if ((Graph::g->nb_vertices - first_freeze_vertex) < (Graph::g->nb_vertices * 0.20)) {
        return;
    }

    auto max_time{std::chrono::high_resolution_clock::now() +
                  std::chrono::seconds(Parameters::p->max_time_local_search)};

    int64_t best_time{0};

    best_solution.reset_tabu();
    Solution working_solution(best_solution);

    long no_improve{1}; // number of iterations without improvement
    int local_best{working_solution.score()};
    long turn{0};
    int force{1}; // perturbation strength

    while (turn < Parameters::p->nb_iter_local_search and
           not Parameters::p->time_limit_reached() and
           not Parameters::p->time_limit_reached(max_time) and
           best_solution.score() != Parameters::p->target) {
        ++turn;
        Solution next_s(working_solution);

        // condition to stop for MCTS when no vertices can be unassigned
        if (not next_s.unassigned_random_heavy_vertices(force, first_freeze_vertex)) {
            return;
        }

        long iter = 0;
        while (next_s.has_unassigned_vertices() and iter < Graph::g->nb_vertices * 10 and
               not Parameters::p->time_limit_reached() and
               not Parameters::p->time_limit_reached(max_time)) {
            ++iter;

            if (next_s.has_unassigned_vertices() and M_1_2_3(next_s, iter)) {
                assert(next_s.check_solution());
                continue;
            }

            std::vector<int> free_vertices = next_s.free_vertices();
            std::shuffle(free_vertices.begin(), free_vertices.end(), rd::generator);
            if (next_s.has_unassigned_vertices() and M_4(next_s, iter, free_vertices)) {
                assert(next_s.check_solution());
                continue;
            }

            if (next_s.has_unassigned_vertices() and M_5(next_s, iter, free_vertices)) {
                assert(next_s.check_solution());
                continue;
            }

            if (next_s.has_unassigned_vertices() and M_6(next_s, iter)) {
                assert(next_s.check_solution());
                continue;
            }
            break;
        }
        if (next_s.get_score_maybe_unassigned() <
            working_solution.get_score_maybe_unassigned()) {
            no_improve = 1;
            working_solution = next_s;
            if (local_best > next_s.get_score_maybe_unassigned()) {
                local_best = next_s.get_score_maybe_unassigned();
            }
            force = 1;
        } else if (no_improve <= Graph::g->nb_vertices) {
            ++no_improve;
            if (force == 3) {
                force = 1;
            } else {
                ++force;
            }
        } else {
            local_best = working_solution.get_score_maybe_unassigned();
            working_solution.perturb_vertices(1);
            no_improve = 1;
        }

        if ((not working_solution.has_unassigned_vertices()) and
            (best_solution.score() > working_solution.score())) {
            best_solution = working_solution;
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

bool M_1_2_3(Solution &solution, const long iter) {
    const int delta{solution.unassigned_score() - solution.score()};
    auto min_vertex{solution.unassigned()[0]};
    int min_cost{Graph::g->nb_vertices};
    int min_c = -1;
    std::vector<int> non_empty_colors = solution.non_empty_colors();
    shuffle(non_empty_colors.begin(), non_empty_colors.end(), rd::generator);
    for (const auto &vertex : solution.unassigned()) {
        const int vertex_weight{Graph::g->weights[vertex]};
        for (const auto &color : non_empty_colors) {
            if (solution.conflicts_colors(color, vertex) == 0 and
                delta > std::max(0, vertex_weight - solution.max_weight(color))) {
                solution.add_vertex_to_color(vertex, color);
                solution.remove_unassigned_vertex(vertex);
                return true;
            }
        }

        std::vector<int> costs(solution.nb_colors(), 0);
        std::vector<int> rellocated(solution.nb_colors(), 0);
        for (const auto &neighbor : Graph::g->neighborhood[vertex]) {
            const int c_neighbor = solution.color(neighbor);
            if (c_neighbor == -1) {
                continue;
            }
            if (delta > std::max(0, vertex_weight - solution.max_weight(c_neighbor))) {

                if (solution.nb_free_colors(neighbor) > 0) {
                    ++rellocated[c_neighbor];
                } else if (solution.tabu[neighbor] < iter) {
                    ++rellocated[c_neighbor];
                    ++costs[c_neighbor];
                }

                if (rellocated[c_neighbor] ==
                        solution.conflicts_colors(c_neighbor, vertex) and
                    costs[c_neighbor] == 0) {
                    std::vector<int> unassigned;
                    for (int y : Graph::g->neighborhood[vertex]) {
                        if (solution.color(y) == c_neighbor) {
                            assert(solution.nb_free_colors(y) > 0);
                            solution.delete_vertex_from_color(y);
                            unassigned.push_back(y);
                        }
                    }

                    solution.add_vertex_to_color(vertex,
                                                 solution.is_color_empty(c_neighbor)
                                                     ? solution.add_new_color()
                                                     : c_neighbor);

                    solution.random_assignment_constrained(unassigned);
                    assert(unassigned.empty());
                    solution.remove_unassigned_vertex(vertex);
                    return true;

                } else if (rellocated[c_neighbor] ==
                               solution.conflicts_colors(c_neighbor, vertex) and
                           costs[c_neighbor] == 1) {
                    if (min_cost > costs[c_neighbor]) {
                        min_cost = costs[c_neighbor];
                        min_c = c_neighbor;
                        min_vertex = vertex;
                    }
                }
            }
        }
    }

    return M_3(solution, iter, min_cost, min_vertex, min_c);
}

bool M_3(Solution &solution,
         const long iter,
         const int min_cost,
         const int vertex,
         const int min_c) {
    if (min_cost == 1) {
        std::vector<int> unassigned;
        for (const auto &y : Graph::g->neighborhood[vertex]) {
            if (solution.color(y) == min_c) {
                if (solution.nb_free_colors(y) > 0) {
                    unassigned.push_back(y);
                } else {
                    solution.add_unassigned_vertex(y);
                }
                solution.delete_vertex_from_color(y);
            }
        }

        solution.add_vertex_to_color(
            vertex, solution.is_color_empty(min_c) ? solution.add_new_color() : min_c);
        solution.tabu[vertex] =
            iter + static_cast<long>(solution.non_empty_colors().size());
        solution.random_assignment_constrained(unassigned);
        assert(unassigned.empty());
        solution.remove_unassigned_vertex(vertex);
        return true;
    }
    return false;
}

bool M_4(Solution &solution, const long iter, const std::vector<int> &free_vertices) {
    const long max_counter{solution.nb_non_empty_colors()};
    int counter = 0;
    for (const auto &v : free_vertices) {
        if (solution.nb_free_colors(v) > 0 and solution.tabu[v] < iter and
            not Graph::g->neighborhood[v].empty() and solution.color(v) != -1) {
            solution.tabu[v] = iter + solution.nb_non_empty_colors();
            solution.random_assignment_constrained(v);
            ++counter;
            if (counter == max_counter) {
                return true;
            }
        }
    }

    return (counter > 0);
}

bool M_5(Solution &solution, const long iter, const std::vector<int> &free_vertices) {
    const int delta{solution.unassigned_score() - solution.score()};

    for (const auto &v : free_vertices) {
        if (solution.nb_free_colors(v) == 0 and solution.tabu[v] < iter and
            not Graph::g->neighborhood[v].empty() and solution.color(v) != -1) {
            std::vector<int> rellocated(solution.nb_colors(), 0);
            for (const auto &neighbor : Graph::g->neighborhood[v]) {
                int c_neighbor = solution.color(neighbor);
                if (c_neighbor == -1)
                    continue;
                if (delta >
                    std::max(0, Graph::g->weights[v] - solution.max_weight(c_neighbor))) {
                    if (solution.nb_free_colors(neighbor) > 0) {
                        ++rellocated[c_neighbor];
                    }
                }
                if (rellocated[c_neighbor] == solution.conflicts_colors(c_neighbor, v)) {
                    std::vector<int> unassigned;
                    for (const auto &y : Graph::g->neighborhood[v]) {
                        if (solution.color(y) == c_neighbor) {
                            if (solution.nb_free_colors(y) > 0) {
                                solution.delete_vertex_from_color(y);
                                unassigned.push_back(y);
                            }
                        }
                    }
                    solution.delete_vertex_from_color(v);
                    solution.tabu[v] = iter + solution.nb_non_empty_colors();
                    solution.add_vertex_to_color(v,
                                                 solution.is_color_empty(c_neighbor)
                                                     ? solution.add_new_color()
                                                     : c_neighbor);

                    solution.random_assignment_constrained(unassigned);
                    return true;
                }
            }
        }
    }
    return false;
}

bool M_6(Solution &solution, const long iter) {
    int min_cost{Graph::g->nb_vertices};
    int min_cost_c = -1;
    const int delta{solution.unassigned_score() - solution.score()};

    const int v = rd::get_random_value(solution.unassigned());
    std::vector<int> rellocated(solution.nb_colors(), 0);
    std::vector<int> costs(solution.nb_colors(), 0);

    for (const auto &neighbor : Graph::g->neighborhood[v]) {
        const int c_neighbor = solution.color(neighbor);
        if (c_neighbor == -1)
            continue;
        if (delta > std::max(0, Graph::g->weights[v] - solution.max_weight(c_neighbor))) {
            if (solution.nb_free_colors(neighbor) > 0) {
                ++rellocated[c_neighbor];
            } else {
                ++rellocated[c_neighbor];
                ++costs[c_neighbor];
            }

            if (rellocated[c_neighbor] == solution.conflicts_colors(c_neighbor, v)) {
                if (min_cost > costs[c_neighbor]) {
                    min_cost_c = c_neighbor;
                    min_cost = costs[c_neighbor];
                }
            }
        }
    }

    if (min_cost_c != -1) {
        std::vector<int> unassigned;
        solution.reset_tabu();
        for (const auto &y : Graph::g->neighborhood[v]) {
            if (solution.color(y) == min_cost_c) {
                if (solution.nb_free_colors(y) > 0) {
                    unassigned.push_back(y);
                } else {
                    solution.add_unassigned_vertex(y);
                }
                solution.delete_vertex_from_color(y);
            }
        }
        solution.tabu[v] = iter + solution.nb_non_empty_colors();
        if (solution.is_color_empty(min_cost_c))
            min_cost_c = solution.add_new_color();
        solution.add_vertex_to_color(v, min_cost_c);

        solution.random_assignment_constrained(unassigned);
        assert(unassigned.empty());
        solution.remove_unassigned_vertex(v);
        return true;
    }
    return false;
}