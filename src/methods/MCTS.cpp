#include "MCTS.h"

#include <cassert>
#include <fstream>
#include <iomanip>
#include <utility>

#include "../utils/random_generator.h"
#include "../utils/utils.h"

MCTS::MCTS()
    : _root_node(nullptr),
      _current_node{_root_node},
      _base_solution(),
      _best_solution(_base_solution),
      _current_solution(_base_solution),
      _turn{0},
      _initialization(get_initialization_fct(Parameters::p->initialization)),
      _local_search(get_local_search_fct(Parameters::p->local_search[0])),
      _simulation(get_simulation_fct(Parameters::p->simulation)) {

    // Creation of the base solution and root node
    std::vector<Action> next_moves{_base_solution.next_possible_moves()};
    assert(next_moves.size() == 1);
    _base_solution.apply_move(next_moves[0]);
    std::vector<Action> next_possible_moves = _base_solution.next_possible_moves();
    _root_node = std::make_shared<Node>(nullptr, next_moves[0], next_possible_moves);

    fmt::print(Parameters::p->output, "{}", header_csv());
}

bool MCTS::stop_condition() const {
    return (_turn < Parameters::p->nb_max_iterations) and
           (not Parameters::p->time_limit_reached()) and
           not(Solution::best_score <= Parameters::p->target) and
           not _root_node->fully_explored();
}

void MCTS::run() {
    while (stop_condition()) {

        _current_node = _root_node;
        _current_solution = _base_solution;

        selection();

        expansion();

        _initialization(_current_solution);

        // if the simulation is depth/fit/depth_fit
        if (_simulation) {
            _simulation(_current_solution, _local_search);
        } else // if the simulation is a simple local search
            if (_local_search) {
            _local_search(_current_solution, false);
        }

        const int score{_current_solution.score()};
        _current_node->update(score);

        if (_best_solution.best_score > score) {
            _t_best = std::chrono::high_resolution_clock::now();
            Solution::best_score = score;
            _best_solution = _current_solution;
            fmt::print(Parameters::p->output, "{}", line_csv());
            _root_node->clean_graph(_best_solution.best_score);
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
        _current_node = rd::get_random_value(next_nodes);
        _current_solution.apply_move(_current_node->move());
    }
}

void MCTS::expansion() {
    const Action next_move{_current_node->next_child()};
    _current_solution.apply_move(next_move);
    const std::vector<Action> next_possible_moves =
        _current_solution.next_possible_moves();
    if (not next_possible_moves.empty()) {
        _current_node =
            std::make_shared<Node>(_current_node.get(), next_move, next_possible_moves);
        _current_node->add_child_to_parent(_current_node);
    }
}

[[nodiscard]] const std::string MCTS::header_csv() const {
    return fmt::format("date,method,instance,{},turn,time,depth,nb total node,nb "
                       "current node,height,{}\n",
                       Parameters::p->header_csv,
                       Solution::header_csv);
}

[[nodiscard]] const std::string MCTS::line_csv() const {
    return fmt::format("{},mcts,{},{},{},{},{},{},{},{},{}\n",
                       get_date_str(),
                       Graph::g->name,
                       Parameters::p->line_csv,
                       _turn,
                       Parameters::p->elapsed_time(_t_best),
                       _current_node->get_depth(),
                       Node::get_total_nodes(),
                       Node::get_nb_current_nodes(),
                       Node::get_height(),
                       _best_solution.line_csv());
}

void MCTS::to_dot(const std::string &file_name) const {
    // fmt::format(
    //     "../graph_dot/{}_{}_{:09}.dot", g->name, _parameters->rand_seed, _turn);
    if ((_turn % 5) == 0) {
        std::ofstream file{file_name};
        file << _root_node->to_dot();
        file.close();
    }
}

simulation_ptr get_simulation_fct(const Simulation &simulation) {
    switch (simulation) {
    case Simulation::fit:
        return fit;
        break;
    case Simulation::depth:
        return depth;
        break;
    case Simulation::depth_fit:
        return depth_fit;
        break;
    default:
        return nullptr;
    }
}

void fit(Solution &solution, const local_search_ptr &local_search) {
    static int fit_condition = std::numeric_limits<int>::max() - 1;
    static std::vector<Solution> past_solutions;
    if (solution.score() <= (fit_condition + 1)) {
        if (ok_distance(solution, past_solutions, Graph::g->nb_vertices / 10)) {
            past_solutions.push_back(solution);
            fit_condition = std::min(solution.score(), fit_condition);
            local_search(solution, false);
        }
    }
}

void depth(Solution &solution, const local_search_ptr &local_search) {
    static std::vector<Solution> past_solutions;
    std::uniform_int_distribution<int> distribution(0, 100);
    if ((solution.get_rank_placed_vertices() * 100) / Graph::g->nb_vertices >=
        distribution(rd::generator)) {
        if (ok_distance(solution, past_solutions, Graph::g->nb_vertices / 10)) {
            past_solutions.push_back(solution);
            local_search(solution, false);
        }
    }
}

void depth_fit(Solution &solution, const local_search_ptr &local_search) {
    static int fit_condition = std::numeric_limits<int>::max() - 1;
    static std::vector<Solution> past_solutions;
    std::uniform_int_distribution<int> distribution(0, 100);
    if (solution.score() <= (fit_condition + 1) and
        (solution.get_rank_placed_vertices() * 100) / Graph::g->nb_vertices <=
            distribution(rd::generator)) {
        if (ok_distance(solution, past_solutions, Graph::g->nb_vertices / 10)) {
            past_solutions.push_back(solution);
            fit_condition = std::min(solution.score(), fit_condition);
            local_search(solution, false);
        }
    }
}
