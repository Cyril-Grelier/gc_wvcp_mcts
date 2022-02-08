#pragma once

#include "../representation/Method.h"
#include "../representation/Node.h"
#include "../representation/Parameters.h"
#include "../representation/Solution.h"
#include "LocalSearch.h"

/** @brief Pointer to simulation function*/
typedef void (*simulation_ptr)(Solution &, const local_search_ptr &);

/**
 * @brief Method for Monte Carlo Tree Search
 *
 */
class MCTS : public Method {
  private:
    /** @brief Root node of the MCTS*/
    std::shared_ptr<Node> _root_node;
    /** @brief Current node*/
    std::shared_ptr<Node> _current_node;
    /** @brief Solution at the beginning of the tree (will be copied at each turn)*/
    Solution _base_solution;
    /** @brief Best found solution*/
    Solution _best_solution;
    /** @brief Current solution*/
    Solution _current_solution;
    /** @brief Current turn of MCTS*/
    long _turn{};
    /** @brief Time before founding best score*/
    std::chrono::high_resolution_clock::time_point _t_best{};
    /** @brief Function to call for initialize the solutions*/
    init_ptr _initialization;
    /** @brief Function to call as local search*/
    local_search_ptr _local_search;
    /** @brief Function to call for simulation*/
    simulation_ptr _simulation;

  public:
    /**
     * @brief Construct a new MCTS object
     *
     */
    explicit MCTS();

    ~MCTS() override = default;

    /**
     * @brief Stopping condition for the MCTS search depending on turn, time limit and the
     * tree fully explored or not
     *
     * @return true continue the search
     * @return false stop the search
     */
    bool stop_condition() const;

    /**
     * @brief Run the 4 phases of MCTS algorithm until stop condition
     *
     */
    void run() override;

    /**
     * @brief Selection phase of the MCTS algorithm
     *
     */
    void selection();

    /**
     * @brief Expansion phase of the MCTS algorithm
     *
     */
    void expansion();

    /**
     * @brief Return string of the MCTS csv format
     *
     * @return std::string header for csv file
     */
    [[nodiscard]] const std::string header_csv() const override;

    /**
     * @brief Return string of a line of the MCTS csv format
     *
     * @return std::string line for csv file
     */
    [[nodiscard]] const std::string line_csv() const override;

    /**
     * @brief Convert the tree in dot format into a file
     *
     * @param file_name name of the file
     */
    void to_dot(const std::string &file_name) const;
};

/**
 * @brief Get the simulation function
 *
 * @param simulation
 * @return simulation_ptr function
 */
simulation_ptr get_simulation_fct(const Simulation &simulation);

/**
 * @brief Apply the greedy on the solution and allow to do a local search if the fitness
 * after greedy is low enough
 *
 * @param solution solution to use, the solution will be modified
 * @param local_search local search to use
 */
void fit(Solution &solution, const local_search_ptr &local_search);

/**
 * @brief Apply the greedy on the solution and allow to do a local search randomly with
 * more chance if the solution is deep in the three
 *
 * @param solution solution to use, the solution will be modified
 * @param local_search local search to use
 */
void depth(Solution &solution, const local_search_ptr &local_search);

/**
 * @brief Apply the greedy on the solution and allow to do a local search randomly with
 * more chance if the solution is deep in the three and if the fitness
 * after greedy is low enough
 *
 * @param solution solution to use, the solution will be modified
 * @param local_search local search to use
 */
void depth_fit(Solution &solution, const local_search_ptr &local_search);
