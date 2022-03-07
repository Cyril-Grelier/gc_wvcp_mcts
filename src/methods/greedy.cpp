#include "greedy.h"

#include "../utils/random_generator.h"

void greedy_random(Solution &solution) {
    for (int vertex{solution.free_vertices().back()}; vertex < Graph::g->nb_vertices;
         ++vertex) {
        std::vector<int> possible_colors;
        for (const int &color : solution.non_empty_colors()) {
            if (solution.conflicts_colors(color, vertex) == 0) {
                possible_colors.emplace_back(color);
            }
        }
        // add -1 to have the possibility to open a new color even its not needed
        possible_colors.emplace_back(-1);
        solution.add_vertex_to_color(vertex, rd::get_random_value(possible_colors));
    }
}

void greedy_deterministic(Solution &solution) {
    for (int vertex{solution.free_vertices().back()}; vertex < Graph::g->nb_vertices;
         ++vertex) {
        solution.add_vertex_to_color(vertex, solution.first_available_color(vertex));
    }
}

void greedy_constrained(Solution &solution) {
    for (int vertex{solution.free_vertices().back()}; vertex < Graph::g->nb_vertices;
         ++vertex) {
        std::vector<int> possible_colors;
        for (const int &color : solution.non_empty_colors()) {
            if (solution.conflicts_colors(color, vertex) == 0) {
                possible_colors.emplace_back(color);
            }
        }
        if (possible_colors.empty()) {
            solution.add_vertex_to_color(vertex, -1);
        } else {
            solution.add_vertex_to_color(vertex, rd::get_random_value(possible_colors));
        }
    }
}

init_ptr get_initialization_fct(const Initialization &initialization) {
    switch (initialization) {
    case Initialization::random:
        return greedy_random;
    case Initialization::constrained:
        return greedy_constrained;
    case Initialization::deterministic:
        return greedy_deterministic;
    default:
        return nullptr;
    }
}
