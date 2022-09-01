#include "greedy.h"

#include "../utils/random_generator.h"

void greedy_random(Solution &solution) {
    while (not solution.unassigned().empty()) {
        const int vertex = solution.next_unassigned();
        auto possible_colors{solution.available_colors(vertex)};
        // add -1 to have the possibility to open a new color even if not needed
        possible_colors.emplace_back(-1);
        solution.add_to_color(vertex, rd::choice(possible_colors));
    }
}

void greedy_constrained(Solution &solution) {
    while (not solution.unassigned().empty()) {
        const int vertex = solution.next_unassigned();
        auto possible_colors{solution.available_colors(vertex)};
        solution.add_to_color(vertex, rd::choice(possible_colors));
    }
}

void greedy_deterministic(Solution &solution) {
    while (not solution.unassigned().empty()) {
        const int vertex = solution.next_unassigned();
        solution.add_to_color(vertex, solution.first_available_color(vertex));
    }
}

void greedy_worst(Solution &solution) {
    while (not solution.unassigned().empty()) {
        const int vertex = solution.next_unassigned();
        solution.add_to_color(vertex, -1);
    }
}

init_ptr get_initialization_fct(const std::string &initialization) {
    if (initialization == "random")
        return greedy_random;
    if (initialization == "constrained")
        return greedy_constrained;
    if (initialization == "deterministic")
        return greedy_deterministic;
    if (initialization == "worst")
        return greedy_worst;
    fmt::print(stderr,
               "Unknown initialization, please select : "
               "random, constrained, deterministic, worst\n");
    exit(1);
}
