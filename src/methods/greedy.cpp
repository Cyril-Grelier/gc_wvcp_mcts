#include "greedy.h"

#include "../utils/random_generator.h"

void greedy_random(Solution &solution) {
    for (int vertex{solution.free_vertices().back()}; vertex < Graph::g->nb_vertices;
         ++vertex) {
        auto possible_colors{solution.available_colors(vertex)};
        // add -1 to have the possibility to open a new color even if not needed
        possible_colors.emplace_back(-1);
        solution.add_to_color(vertex, rd::choice(possible_colors));
    }
}

void greedy_constrained(Solution &solution) {
    for (int vertex{solution.free_vertices().back()}; vertex < Graph::g->nb_vertices;
         ++vertex) {
        auto possible_colors{solution.available_colors(vertex)};
        solution.add_to_color(vertex, rd::choice(possible_colors));
    }
}

void greedy_deterministic(Solution &solution) {
    for (int vertex{solution.free_vertices().back()}; vertex < Graph::g->nb_vertices;
         ++vertex) {
        solution.add_to_color(vertex, solution.first_available_color(vertex));
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
