#pragma once

#include <memory>

#include "Action.h"
#include "Graph.h"
#include "Parameters.h"
#include <tuple>

/**
 * @brief Representation of a solution for Weighted Vertex Coloring Problem
 *
 */
class Solution {
  public:
    /** @brief WVCP best found score*/
    static int best_score;
    /** @brief Minimal nb of color found (you have to update it)*/
    static int best_nb_colors;
    /** @brief Tabu list*/
    std::vector<long> tabu{};

    /** @brief Header csv*/
    const static std::string header_csv;

  private:
    /** @brief For each vertex, its color*/
    std::vector<int> _colors{};
    /** @brief For each color, list of the vertices colored with the color, sorted*/
    std::vector<std::vector<int>> _colors_vertices{};
    /** @brief For each color, list of the weights of the vertices colored with the color,
     * sorted*/
    std::vector<std::vector<int>> _colors_weights{};
    /** @brief For each color, for each vertex, number of neighbors in the color*/
    std::vector<std::vector<int>> _conflicts_colors{};
    /** @brief For each color, first vertices are with the color (until color size) last
     * without*/
    std::vector<std::vector<int>> _color_partition{};
    /** @brief For each color, for each vertex, position of the vertex in the color
     * partition*/
    std::vector<std::vector<int>> _position{};
    /** @brief For each vertex, number of color the vertex can take without increasing the
     * score*/
    std::vector<int> _nb_free_colors{};
    /** @brief List of unused colors*/
    std::vector<int> _empty_colors{};
    /** @brief List of used colors*/
    std::vector<int> _non_empty_colors{};
    /** @brief List of vertices to color*/
    std::vector<int> _free_vertices{};
    /** @brief List of unassigned vertices*/
    std::vector<int> _unassigned{};
    /** @brief number of openned colors (not automaticaly all used)*/
    int _nb_colors{};
    /** @brief WVCP score*/
    int _score{};
    /** @brief WVCP Score when some vertices are unassigned*/
    int _unassigned_score{};
    /** @brief number of conflicts in the current solution*/
    int _nb_conflicts;
    /** @brief list of conflicting edges in the solution*/
    std::vector<std::tuple<int, int>> _conflict_edges;
    /** @brief edge weights (for RedLS)*/
    std::vector<std::vector<int>> _edge_weights;

  public:
    /**
     * @brief Construct a new Solution object
     *
     */
    Solution();

    /**
     * @brief Destroy the Solution object
     *
     */
    ~Solution() = default;

    /**
     * @brief Give the first available color for the given vertex,
     * -1 if no color available
     *
     * @param vertex the vertex
     * @return int the first available color
     */
    [[nodiscard]] int first_available_color(const int &vertex);

    /**
     * @brief Delete heaviest vertices from force colors
     *
     * @param force Number of deletions
     * @param first_freeze_vertex if some vertex cannot be moved
     * @return true if ok
     * @return false if all deletions didn't happened
     */
    bool unassigned_random_heavy_vertices(const int force, const int first_freeze_vertex);

    /**
     * @brief Do a grenade operator on force random vertices
     *
     * @param force number of time the grenade operator is applied
     */
    void perturb_vertices(const int force);

    /**
     * @brief Color the vertex to the color
     * if the color is not created, the color is created
     * if the vertex is already colored, the vertex is uncolored before colored
     * with the color
     *
     * @param vertex the verter to color
     * @param color the color to use
     * @return int the color used (may be different from the given color if the color was
     * empty)
     */
    int add_vertex_to_color(const int vertex, const int color);

    /**
     * @brief Remove color of a vertex
     *
     * @param vertex vertex to modify
     * @return int the old color
     */
    int delete_vertex_from_color(const int vertex);

    /**
     * @brief Gives a new empty color, add prepare all structures
     *
     * @return int the new color
     */
    [[nodiscard]] int add_new_color();

    /**
     * @brief Assigned vertices to color randomly
     *
     * @param vertices vertices to color, the list is modified
     */
    void random_assignment(std::vector<int> &vertices);

    /**
     * @brief Assigned color to vertex randomly without increasing the score
     *
     * @param vertex the vertex to color
     * @return true the vertex has been colored without increasing the score
     * @return false there is no color available to color the vertex without increasing
     * the score
     */
    bool random_assignment_constrained(const int vertex);

    /**
     * @brief Assigned color to the vertices randomly without increasing the score
     *
     * @param vertices vertices to color, the list is modified, if the list isn't empty
     * after the function, the vertices left haven't free colors
     */
    void random_assignment_constrained(std::vector<int> &vertices);

    /**
     * @brief Give the next possible moves with the current placement of vertices
     *
     * @return std::vector<Action> List of next moves
     */
    [[nodiscard]] std::vector<Action> next_possible_moves();

    /**
     * @brief Apply a move to the solution
     *
     * @param mv move to apply
     */
    void apply_move(const Action &mv);

    /**
     * @brief Check the validity of the solution
     *
     * @return true the solution is valid
     * @return false the solution is invalid
     */
    bool check_solution() const;

    /**
     * @brief Init and reset tabu list
     *
     */
    void reset_tabu();

    /**
     * @brief Init and reset edge weight matrix (for RedLS)
     *
     */
    void reset_edge_weights();

    /**
     * @brief Increment edge weight matrix (for RedLS)
     *
     */
    void increment_edge_weights();

    /**
     * @brief Compute the difference on the score if the vertex is colored with the color
     *
     * @param vertex the vertex to color
     * @param color the color to use
     * @return int difference on the score
     */
    [[nodiscard]] int get_delta_score(const int vertex, const int color) const;

    /**
     * @brief Compute the difference on the score if the vertex lost its color
     *
     * @param vertex the vertex to check
     * @return int the difference on the score
     */
    [[nodiscard]] int get_delta_old_color(const int vertex) const;

    /**
     * @brief Gives the score of the solution or unassigned score if incomplete
     *
     * @return int the score maybe unassigned
     */
    [[nodiscard]] int get_score_maybe_unassigned() const;

    /**
     * @brief Return unassigned vertices
     *
     * @return const std::vector<int>& unassigned vertices
     */
    [[nodiscard]] const std::vector<int> &unassigned() const;

    /**
     * @brief Return the number of color where the vertex can be moved without
     * incrementing the score
     *
     * @param vertex the vertex
     * @return int the number of color free
     */
    [[nodiscard]] int nb_free_colors(const int &vertex) const;

    /**
     * @brief Gives the score of the solution
     *
     * @return int the score
     */
    [[nodiscard]] int score() const;

    /**
     * @brief Gives the unassigned score of the solution
     *
     * @return int the unassigned score
     */
    [[nodiscard]] int unassigned_score() const;

    /**
     * @brief Get the number of the last placed vertex
     *
     * @return int the number of the last placed vertex
     */
    [[nodiscard]] int get_rank_placed_vertices() const;

    /**
     * @brief Return is there is or not unassigned vertices
     *
     * @return true there is no unassigned vertices
     * @return false there is unassigned vertices
     */
    [[nodiscard]] bool has_unassigned_vertices() const;

    /**
     * @brief Add a vertex to the unassigned list
     *
     * @param vertex vertex to add
     */
    void add_unassigned_vertex(const int &vertex);

    /**
     * @brief Remove the given vertex from the unassigned list
     *
     * @param vertex vertex to remove
     */
    void remove_unassigned_vertex(const int &vertex);

    /**
     * @brief Return the colors of the vertices
     *
     * @return const std::vector<int> colors
     */
    [[nodiscard]] const std::vector<int> &colors() const;

    /**
     * @brief Get the number of non_empty colors
     *
     * @return long number of colors currently used
     */
    [[nodiscard]] long nb_non_empty_colors() const;

    /**
     * @brief Return non empty colors
     *
     * @return const std::vector<int>& non empty colors
     */
    [[nodiscard]] const std::vector<int> &non_empty_colors() const;

    /**
     * @brief Return number of colors
     *
     * @return int number of colors
     */
    [[nodiscard]] int nb_colors() const;

    /**
     * @brief Return the free_vertices list
     *
     * @return const std::vector<int>& the list
     */
    [[nodiscard]] const std::vector<int> &free_vertices() const;

    /**
     * @brief return the number of conflicts on the color for the vertex
     *
     * @return int number of conflicts
     */
    [[nodiscard]] int conflicts_colors(const int &color, const int &vertex) const;

    /**
     * @brief Return number of conflicts
     *
     * @return int number of conflicts
     */
    [[nodiscard]] int nb_conflicts() const;

    /**
     * @brief Return the list of conflicting edges
     *
     * @return list of conflicting edges (tuples)
     */
    [[nodiscard]] std::vector<std::tuple<int, int>> conflict_edges() const;

    /**
     * @brief Return whether the vertex has conflict or not
     *
     * @param vertex the vertex
     * @return true there is conflicts
     * @return false there is no conflicts
     */
    [[nodiscard]] bool has_conflicts(const int vertex) const;

    /**
     * @brief Compute the difference on the conflicts if the vertex is colored with the
     * color
     *
     * @param vertex the vertex to use
     * @param color the color to use
     * @return int the difference on the number of conflicts
     */
    [[nodiscard]] int get_delta_conflicts(const int vertex, const int color) const;

    /**
     * @brief Close colors and push unassigned vertices other colors and create conflicts
     *
     * @param nb_total_color Number of color the solution will have
     */
    void reduce_nb_colors(const int nb_total_color);

    /**
     * @brief Return number of vertices per color
     *
     * @param nb_colors_max the vector size will be of size nb_color_max
     * @return std::vector<int> number of vertices per color
     */
    [[nodiscard]] std::vector<int> nb_vertices_per_color(const int nb_colors_max) const;

    /**
     * @brief Return the color of a vertex
     *
     * @param vertex the vertex
     * @return int the color of the vertex
     */
    [[nodiscard]] int color(const int &vertex) const;

    /**
     * @brief Return vertices in given color
     *
     * @param color given color
     * @return std::vector<int> vertices in the color
     */
    [[nodiscard]] const std::vector<int> &colors_vertices(const int &color) const;

    /**
     * @brief Return colors weights
     *
     * @return const std::vector<std::vector<int>>& colors_weights of the solution
     */
    [[nodiscard]] const std::vector<std::vector<int>> &colors_weights() const;

    /**
     * @brief Return max weight of the color
     *
     * @param color the color
     * @return int the max weight
     */
    [[nodiscard]] int max_weight(const int &color) const;

    /**
     * @brief Return true if the color is empty
     *
     * @return true no vertex in the color
     * @return false vertex in the color
     */
    [[nodiscard]] bool is_color_empty(const int color) const;

    /**
     * @brief Return the solution in csv format
     *
     * @return std::string the solution in csv format
     */
    [[nodiscard]] std::string line_csv() const;
};

/**
 * @brief Compute an approximation of the distance between two solutions
 *
 * @param sol1 first solution
 * @param sol2 second solution
 * @return int distance
 */
[[nodiscard]] int distance_approximation(const Solution &sol1, const Solution &sol2);

/**
 * @brief Compute the distance between two solutions
 *
 * @param sol1 first solution
 * @param sol2 second solution
 * @return int distance
 */
[[nodiscard]] int distance(const Solution &sol1, const Solution &sol2);

/**
 * @brief Check if the solution is at least at a distance (approximation) of distance_max
 * from other solutions
 *
 * @param current_solution solution to check
 * @param past_solutions other solutions
 * @param distance_max max distance
 * @return true it is
 * @return false it is not
 */
[[nodiscard]] bool ok_distance(const Solution &current_solution,
                               const std::vector<Solution> &past_solutions,
                               const int distance_max);
