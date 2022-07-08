#include "MCTS.h"

#include <cassert>
#include <fstream>
#include <iomanip>
#include <utility>

#include "../utils/random_generator.h"
#include "../utils/utils.h"

simulation_ptr get_simulation_fct(const std::string &simulation) {
    if (simulation == "fit") {
        return fit;
    } else if (simulation == "depth") {
        return depth;
    } else if (simulation == "depth_fit") {
        return depth_fit;
    }
    return nullptr;
}

MCTS::MCTS()
    : _root_node(nullptr),
      _current_node{_root_node},
      _base_solution(),
      _best_solution(),
      _current_solution(_base_solution),
      _turn{0},
      _initialization(get_initialization_fct(Parameters::p->initialization)),
      _local_search(get_local_search_fct(Parameters::p->local_search)),
      _simulation(get_simulation_fct(Parameters::p->simulation)) {
    greedy_worst(_best_solution);

    if (Parameters::p->use_target and Parameters::p->target > 0) {
        Solution::best_score_wvcp = Parameters::p->target;
    }
    // Creation of the base solution and root node
    const auto next_moves{next_possible_moves(_base_solution)};
    assert(next_moves.size() == 1);
    apply_action(_base_solution, next_moves[0]);
    const auto next_possible_actions{next_possible_moves(_base_solution)};
    _root_node = std::make_shared<Node>(nullptr, next_moves[0], next_possible_actions);

    fmt::print(Parameters::p->output, "{}", header_csv());
}

bool MCTS::stop_condition() const {
    return (_turn < Parameters::p->nb_max_iterations) and
           (not Parameters::p->time_limit_reached()) and
           not(Parameters::p->objective == "reached" and
               (_best_solution.score_wvcp() <= Parameters::p->target)) and
           not _root_node->fully_explored();
}

void MCTS::run() {
    SimulationHelper helper;
    while (stop_condition()) {

        _current_node = _root_node;
        _current_solution = _base_solution;

        selection();

        expansion();

        _initialization(_current_solution);

        // Doesn't perform local search if less than 10%
        // of the vertices are free to be moved
        const bool can_perform_ls{
            (Graph::g->nb_vertices - _current_solution.first_free_vertex()) >
            (Graph::g->nb_vertices * 0.10)};
        // if the simulation is depth/fit/depth_fit
        if (_simulation and can_perform_ls) {
            _simulation(_current_solution, _local_search, helper);
        } else // if the simulation is a simple local search
            if (_local_search and can_perform_ls) {
            _local_search(_current_solution, false);
        }

        const int score_wvcp{_current_solution.score_wvcp()};
        _current_node->update(score_wvcp);

        if (_best_solution.score_wvcp() > score_wvcp) {
            _t_best = std::chrono::high_resolution_clock::now();
            _best_solution = _current_solution;
            if (Solution::best_score_wvcp > score_wvcp)
                Solution::best_score_wvcp = score_wvcp;
            fmt::print(Parameters::p->output, "{}", line_csv());
            _root_node->clean_graph(_best_solution.score_wvcp());
        }
        ++_turn;
    }
    fmt::print(Parameters::p->output, "{}", line_csv());
}

void MCTS::selection() {
    while (not _current_node->terminal()) {
        double max_score{std::numeric_limits<double>::min()};
        std::vector<std::shared_ptr<Node>> next_nodes;
        for (const auto &node : _current_node->children_nodes()) {
            if (node->score_ucb() > max_score) {
                max_score = node->score_ucb();
                next_nodes = {node};
            } else if (node->score_ucb() == max_score) {
                next_nodes.push_back(node);
            }
        }
        _current_node = rd::choice(next_nodes);
        apply_action(_current_solution, _current_node->move());
    }
}

void MCTS::expansion() {
    const Action next_move{_current_node->next_child()};
    apply_action(_current_solution, next_move);
    const auto next_possible_actions{next_possible_moves(_current_solution)};
    if (not next_possible_actions.empty()) {
        _current_node =
            std::make_shared<Node>(_current_node.get(), next_move, next_possible_actions);
        _current_node->add_child_to_parent(_current_node);
    }
}

[[nodiscard]] const std::string MCTS::header_csv() const {
    return fmt::format("date,{},turn,time,depth,nb total node,nb "
                       "current node,height,{}\n",
                       Parameters::p->header_csv,
                       Solution::header_csv);
}

[[nodiscard]] const std::string MCTS::line_csv() const {
    return fmt::format("{},{},{},{},{},{},{},{},{}\n",
                       get_date_str(),
                       Parameters::p->line_csv,
                       _turn,
                       Parameters::p->elapsed_time(_t_best),
                       _current_node->get_depth(),
                       Node::get_total_nodes(),
                       Node::get_nb_current_nodes(),
                       Node::get_height(),
                       _best_solution.line_csv());
}

std::vector<Action> next_possible_moves(const Solution &solution) {
    std::vector<Action> moves;
    if (solution.free_vertices().empty()) {
        return moves;
    }
    const int next_vertex = solution.first_free_vertex();
    for (const auto color : solution.non_empty_colors()) {
        if (solution.conflicts_colors(color, next_vertex) == 0) {
            const int next_score =
                solution.score_wvcp() + solution.delta_wvcp_score(next_vertex, color);
            if (Solution::best_score_wvcp > next_score) {
                moves.emplace_back(Action{next_vertex, color, next_score});
            }
        }
    }
    const int next_score = solution.score_wvcp() + Graph::g->weights[next_vertex];
    if (Solution::best_score_wvcp > next_score) {
        moves.emplace_back(Action{next_vertex, -1, next_score});
    }
    std::sort(moves.begin(), moves.end(), compare_actions);
    return moves;
}

void apply_action(Solution &solution, const Action &action) {
    solution.add_to_color(action.vertex, action.color);
    assert(solution.first_free_vertex() == action.vertex);
    solution.pop_first_free_vertex();
    assert(solution.score_wvcp() == action.score);
}

void MCTS::to_dot(const std::string &file_name) const {
    if ((_turn % 5) == 0) {
        std::ofstream file{file_name};
        file << _root_node->to_dot();
        file.close();
    }
}

void fit(Solution &solution,
         const local_search_ptr &local_search,
         SimulationHelper &helper) {
    if (solution.score_wvcp() <= (helper.fit_condition + 1)) {
        if (std::all_of(helper.past_solutions.begin(),
                        helper.past_solutions.end(),
                        [solution](Solution sol) {
                            return distance_approximation(sol, solution) >
                                   Graph::g->nb_vertices / 10;
                        })) {
            helper.past_solutions.push_back(solution);
            helper.fit_condition = std::min(solution.score_wvcp(), helper.fit_condition);
            local_search(solution, false);
        }
    }
}

void depth(Solution &solution,
           const local_search_ptr &local_search,
           SimulationHelper &helper) {
    std::uniform_int_distribution<int> distribution(0, 100);
    if ((solution.first_free_vertex() * 100) / Graph::g->nb_vertices >=
        distribution(rd::generator)) {
        if (std::all_of(helper.past_solutions.begin(),
                        helper.past_solutions.end(),
                        [solution](Solution sol) {
                            return distance_approximation(sol, solution) >
                                   Graph::g->nb_vertices / 10;
                        })) {
            helper.past_solutions.push_back(solution);
            local_search(solution, false);
        }
    }
}

void depth_fit(Solution &solution,
               const local_search_ptr &local_search,
               SimulationHelper &helper) {
    std::uniform_int_distribution<int> distribution(0, 100);
    if (solution.score_wvcp() <= (helper.fit_condition + 1) and
        (solution.first_free_vertex() * 100) / Graph::g->nb_vertices <=
            distribution(rd::generator)) {
        if (std::all_of(helper.past_solutions.begin(),
                        helper.past_solutions.end(),
                        [solution](Solution sol) {
                            return distance_approximation(sol, solution) >
                                   Graph::g->nb_vertices / 10;
                        })) {
            helper.past_solutions.push_back(solution);
            helper.fit_condition = std::min(solution.score_wvcp(), helper.fit_condition);
            local_search(solution, false);
        }
    }
}
