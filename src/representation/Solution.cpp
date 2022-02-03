#include "Solution.h"

#include <algorithm>
#include <cassert>
#include <numeric>

#include "../utils/random_generator.h"
#include "../utils/utils.h"

int Solution::best_score = std::numeric_limits<int>::max();
int Solution::best_nb_colors = std::numeric_limits<int>::max();

const std::string Solution::header_csv = "nb_colors,nb_conflicts,score,solution";

Solution::Solution()
    : _colors(Graph::g->nb_vertices, -1),
      _colors_vertices(),
      _colors_weights(),
      _conflicts_colors(),
      _color_partition(),
      _position(),
      _nb_free_colors(Graph::g->nb_vertices, 0),
      _empty_colors(),
      _non_empty_colors(),
      _free_vertices(Graph::g->nb_vertices),
      _unassigned{},
      _score{},
      _unassigned_score{},
      _nb_conflicts{0},
      _conflict_edges(),
      _edge_weights(Graph::g->nb_vertices, std::vector<int>(Graph::g->nb_vertices, 0)) {
    // init _free_vertices
    int n{Graph::g->nb_vertices};
    std::generate(_free_vertices.begin(), _free_vertices.end(), [&n] {
        return --n;
    });

    // init _edge_weights
    for (const auto &[v1, v2] : Graph::g->edges_list) {
        _edge_weights[v1][v2] = 1;
        _edge_weights[v2][v1] = 1;
    }
}

[[nodiscard]] int Solution::first_available_color(const int &vertex) {
    for (const int &color : _non_empty_colors) {
        if (_conflicts_colors[color][vertex] == 0 and
            not _colors_weights[color].empty()) {
            return color;
        }
    }
    return -1;
}

bool Solution::unassigned_random_heavy_vertices(const int force,
                                                const int first_freeze_vertex = -1) {
    std::uniform_int_distribution<int> distribution(0, _nb_colors - 1);
    std::vector<int> unassigned;
    _unassigned_score = _score;
    for (int i = 0; i < force; ++i) {

        std::vector<int> possible_colors;
        for (const auto &color : _non_empty_colors) {
            if (std::all_of(_colors_vertices[color].begin(),
                            _colors_vertices[color].end(),
                            [first_freeze_vertex](const int &vertex) {
                                return first_freeze_vertex < vertex;
                            }))
                possible_colors.push_back(color);
        }

        if (possible_colors.empty()) {
            return false;
        }

        const int color{rd::get_random_value(possible_colors)};
        // const int color{rd::get_random_value(_non_empty_colors)};

        const int max_weight = _colors_weights[color].back();

        std::vector<int> to_unassign;
        for (const auto &vertex : _colors_vertices[color]) {
            if (Graph::g->weights[vertex] == max_weight) {
                to_unassign.push_back(vertex);
            }
        }
        for (const auto &vertex : to_unassign) {
            delete_vertex_from_color(vertex);
            unassigned.push_back(vertex);
        }
    }

    std::shuffle(unassigned.begin(), unassigned.end(), rd::generator);
    std::shuffle(_non_empty_colors.begin(), _non_empty_colors.end(), rd::generator);
    random_assignment_constrained(unassigned);

    std::copy(unassigned.begin(), unassigned.end(), std::back_inserter(_unassigned));
    return true;
}

void Solution::perturb_vertices(const int force) {
    assert(_unassigned.empty());
    std::uniform_int_distribution<int> distribution_v(0, Graph::g->nb_vertices - 1);
    std::uniform_int_distribution<int> distribution_c(0, _nb_colors - 1);

    for (int i = 0; i < force; ++i) {
        int color;
        int vertex;
        do {
            vertex = distribution_v(rd::generator);
            color = distribution_c(rd::generator);
        } while (Graph::g->neighborhood[vertex].empty() || _colors[vertex] == color ||
                 _colors_vertices[color].empty());

        delete_vertex_from_color(vertex);

        std::vector<int> unassigned;
        for (const int neighbor : Graph::g->neighborhood[vertex]) {
            if (_colors[neighbor] == color) {
                unassigned.push_back(neighbor);
                delete_vertex_from_color(neighbor);
            }
        }

        if (_colors_vertices[color].empty()) {
            add_vertex_to_color(vertex, add_new_color());
        } else {
            add_vertex_to_color(vertex, color);
        }
        std::random_shuffle(unassigned.begin(), unassigned.end());

        random_assignment_constrained(unassigned);
        if (!unassigned.empty()) {
            random_assignment(unassigned);
        }
    }
}

int Solution::add_vertex_to_color(const int vertex, const int color_proposed) {
    assert(vertex < Graph::g->nb_vertices);
    assert(_colors[vertex] == -1);

    // if the new color is "null" or in empty colors
    // (the color is empty and the last non empty color is not the color), add new color
    const int color{color_proposed == -1 or _nb_colors == color_proposed or
                            (_colors_vertices[color_proposed].empty() and
                             _non_empty_colors.back() != color_proposed)
                        ? add_new_color()
                        : color_proposed};

    const int old_max_weight =
        _colors_weights[color].empty() ? 0 : _colors_weights[color].back();

    // Added for RedLS - update nb of conflicts and list of conflicting edges
    if (_conflicts_colors[color][vertex] > 0) {

        // Update conflict score
        _nb_conflicts += _conflicts_colors[color][vertex];

        for (const auto &neighbor : Graph::g->neighborhood[vertex]) {
            if (_colors[neighbor] == color) {
                _conflict_edges.emplace_back(std::make_tuple(neighbor, vertex));
                _conflict_edges.emplace_back(std::make_tuple(vertex, neighbor));
            }
        }
    }

    // update conflicts and free colors for neighbors
    for (const auto &neighbor : Graph::g->neighborhood[vertex]) {

        //++_conflicts_colors[color][neighbor];
        _conflicts_colors[color][neighbor] += _edge_weights[vertex][neighbor];

        if (_conflicts_colors[color][neighbor] == 1 and
            Graph::g->weights[neighbor] <= old_max_weight) {
            _nb_free_colors[neighbor]--;
        }
    }

    // update position and color partition
    const int color_size = static_cast<int>(_colors_weights[color].size());
    // current position of v in the color c partition vector
    const int pos_v = _position[color][vertex];
    // new position of v in the color c partition vector
    const int new_pos_v = color_size;
    // first vertex of the second partition in the color c partition vector
    const auto vertex2 = _color_partition[color][color_size];
    // ensures v is in the first partition of the color c partition vector
    assert(color_size <= pos_v);
    // swap v with the first vertex of the color c partition vector
    std::swap(_color_partition[color][pos_v], _color_partition[color][new_pos_v]);
    _position[color][vertex] = new_pos_v;
    _position[color][vertex2] = pos_v;

    // update weights and vertices for the color class
    insert_sorted(_colors_weights[color], Graph::g->weights[vertex]);
    insert_sorted(_colors_vertices[color], vertex);

    // update score and free vector
    // if the vertex increase the class weight
    if (Graph::g->weights[vertex] > old_max_weight) {
        // for all vertices outside the color
        for (int i = static_cast<int>(_colors_weights[color].size());
             i < Graph::g->nb_vertices;
             ++i) {
            const int v_c = _color_partition[color][i];
            // check if the outside vertex is heavier than the last max weight but
            // lower than the new max weight
            if (Graph::g->weights[v_c] > old_max_weight and
                Graph::g->weights[v_c] <= Graph::g->weights[vertex] and
                _conflicts_colors[color][v_c] == 0) {
                // update its number of free colors
                ++_nb_free_colors[v_c];
            }
        }
        // update score
        _score += (Graph::g->weights[vertex] - old_max_weight);
    } else {
        // the vertex lost a free color
        _nb_free_colors[vertex]--;
    }
    // update colors
    _colors[vertex] = color;
    return color;
}

int Solution::delete_vertex_from_color(const int vertex) {
    const int color = _colors[vertex];
    assert(color != -1);
    assert(color < _nb_colors);
    assert(vertex < Graph::g->nb_vertices);

    // Added for RedLS - update nb of conflicts and list of conflicting edges
    if (_conflicts_colors[color][vertex] > 0) {
        // Update conflict score
        _nb_conflicts -= _conflicts_colors[color][vertex];
        for (const auto &neighbor : Graph::g->neighborhood[vertex]) {
            if (_colors[neighbor] == color) {
                int index{0};
                int index1{-1};
                int index2{-1};
                for (const auto &[edge1, edge2] : _conflict_edges) {
                    if (edge1 == neighbor && edge2 == vertex) {
                        index1 = index;
                    }
                    if (edge1 == vertex && edge2 == neighbor) {
                        index2 = index;
                    }
                    index += 1;
                }
                int indexMin = std::min(index1, index2);
                _conflict_edges.erase(_conflict_edges.begin() + indexMin);
                _conflict_edges.erase(_conflict_edges.begin() + indexMin);
            }
        }
    }

    // update conflicts and free color for neighbors
    for (const int neighbor : Graph::g->neighborhood[vertex]) {
        // _conflicts_colors[color][neighbor]--;
        _conflicts_colors[color][neighbor] -= _edge_weights[vertex][neighbor];

        if (_conflicts_colors[color][neighbor] == 0 and
            Graph::g->weights[neighbor] <= _colors_weights[color].back()) {
            ++_nb_free_colors[neighbor];
        }
    }

    const int color_size = static_cast<int>(_colors_weights[color].size());
    // current position of v in the color c colors_ vector
    const int pos_v = _position[color][vertex];
    // new position of v in the color c colors_ vector
    const int new_pos_v = color_size - 1;
    // last vertex of the first partition in the color c colors_ vector
    const int vertex2 = _color_partition[color][color_size - 1];
    // ensures v is in the solution partition of the solution vector
    assert(pos_v < color_size);
    // swap v with the last vertex of the second partition
    std::swap(_color_partition[color][pos_v], _color_partition[color][new_pos_v]);
    _position[color][vertex] = new_pos_v;
    _position[color][vertex2] = pos_v;
    // change the boundary between the blocks to make v the first vertex of the
    // second partition
    const int old_weight{_colors_weights[color].back()};
    const int vertex_weight{Graph::g->weights[vertex]};

    _score += get_delta_old_color(vertex);
    erase_sorted(_colors_weights[color], vertex_weight);
    erase_sorted(_colors_vertices[color], vertex);
    const int max_weight_color =
        _colors_weights[color].empty() ? 0 : _colors_weights[color].back();
    // update free colors
    if (vertex_weight == old_weight) {
        for (int i = static_cast<int>(_colors_weights[color].size());
             i < Graph::g->nb_vertices;
             ++i) {
            const int v_c = _color_partition[color][i];
            if (Graph::g->weights[v_c] <= old_weight and
                Graph::g->weights[v_c] > max_weight_color and
                _conflicts_colors[color][v_c] == 0 and v_c != vertex) {
                _nb_free_colors[v_c]--;
            }
        }
    }

    if (old_weight == max_weight_color) {
        ++_nb_free_colors[vertex];
    }
    _colors[vertex] = -1;

    // delete color if needed
    if (_colors_weights[color].empty()) {
        const auto it =
            std::find(_non_empty_colors.begin(), _non_empty_colors.end(), color);
        _non_empty_colors[std::distance(_non_empty_colors.begin(), it)] =
            _non_empty_colors.back();
        _non_empty_colors.pop_back();

        _empty_colors.push_back(color);
    }
    return color;
}

[[nodiscard]] int Solution::add_new_color() {
    if (_empty_colors.empty()) {
        _color_partition.emplace_back(Graph::g->nb_vertices);
        _position.emplace_back(Graph::g->nb_vertices);
        _conflicts_colors.emplace_back(Graph::g->nb_vertices, 0);
        _colors_vertices.emplace_back();
        _colors_weights.emplace_back();

        std::iota(
            _color_partition[_nb_colors].begin(), _color_partition[_nb_colors].end(), 0);
        std::iota(_position[_nb_colors].begin(), _position[_nb_colors].end(), 0);

        _non_empty_colors.push_back(_nb_colors);
        return _nb_colors++;
    } else {
        const int color = _empty_colors.back();
        _empty_colors.pop_back();
        _non_empty_colors.push_back(color);
        return color;
    }
}

void Solution::random_assignment(std::vector<int> &vertices) {
    while (!vertices.empty()) {
        const auto vertex = vertices.back();
        vertices.pop_back();
        std::vector<int> possible_colors;
        for (const int &color : _non_empty_colors) {
            if (_conflicts_colors[color][vertex] == 0 and
                not _colors_weights[color].empty()) {
                possible_colors.emplace_back(color);
            }
        }
        if (possible_colors.empty()) {
            add_vertex_to_color(vertex, add_new_color());
        } else {
            add_vertex_to_color(vertex, rd::get_random_value(possible_colors));
        }
    }
}

bool Solution::random_assignment_constrained(const int vertex) {
    std::uniform_int_distribution<int> distribution(0, _nb_colors - 1);
    int color = distribution(rd::generator);

    for (int i = 0; i < _nb_colors; ++i) {
        if (_conflicts_colors[color][vertex] == 0 and
            not _colors_weights[color].empty() and
            _colors_weights[color].back() >= Graph::g->weights[vertex] and
            color != _colors[vertex]) {
            if (_colors[vertex] != -1) {
                delete_vertex_from_color(vertex);
            }
            add_vertex_to_color(vertex, color);
            return true;
        }
        color = color == _nb_colors - 1 ? 0 : color + 1;
    }
    return false;
}

void Solution::random_assignment_constrained(std::vector<int> &vertices) {
    std::vector<int> unassigned;

    for (const auto &vertex : vertices) {
        if (!random_assignment_constrained(vertex)) {
            unassigned.push_back(vertex);
        }
    }
    vertices = unassigned;
}

[[nodiscard]] std::vector<Action> Solution::next_possible_moves() {
    std::vector<Action> moves;
    if (_free_vertices.empty()) {
        return moves;
    }
    const int next_vertex = _free_vertices.back();
    for (const auto color : _non_empty_colors) {
        if (_conflicts_colors[color][next_vertex] == 0) {
            const int next_score = _score + get_delta_score(next_vertex, color);
            if (best_score > next_score) {
                moves.emplace_back(Action{next_vertex, color, next_score});
            }
        }
    }
    const int color = _empty_colors.empty() ? _nb_colors : _non_empty_colors.back();
    const int next_score = _score + get_delta_score(next_vertex, color);
    if (best_score > next_score) {
        moves.emplace_back(Action{next_vertex, color, next_score});
    }
    std::sort(moves.begin(), moves.end(), compare_actions);
    return moves;
}

void Solution::apply_move(const Action &mv) {
    add_vertex_to_color(mv.vertex, mv.color == _nb_colors ? add_new_color() : mv.color);
    assert(_free_vertices.back() == mv.vertex);
    _free_vertices.pop_back();
    assert(_score == mv.score);
}

bool Solution::check_solution() const {
    std::vector<int> max_colors_weights(_nb_colors, 0);
    int score = 0;

    for (int vertex = 0; vertex < Graph::g->nb_vertices; ++vertex) {
        if (Graph::g->neighborhood[vertex].empty() or _colors[vertex] == -1)
            continue;
        const int color = _colors[vertex];

        if (max_colors_weights[color] < Graph::g->weights[vertex]) {
            max_colors_weights[color] = Graph::g->weights[vertex];
        }

        assert(color >= 0 and color < _nb_colors and
               _conflicts_colors[color][vertex] == 0);

        for (const int &neighbor : Graph::g->neighborhood[vertex]) {
            (void)neighbor;
            assert(color != _colors[neighbor]);
        }

        int free = 0;
        for (int col = 0; col < _nb_colors; ++col) {
            if (_conflicts_colors[col][vertex] == 0 and col != color and
                not _colors_weights[col].empty() and
                Graph::g->weights[vertex] <= _colors_weights[col].back())
                ++free;
        }
        assert(_nb_free_colors[vertex] == free);
    }

    for (int color = 0; color < _nb_colors; ++color) {
        if (_colors_weights[color].empty())
            continue;

        assert(_colors_weights[color].back() == max_colors_weights[color]);
        score += max_colors_weights[color];

        for (int vertex = 0; vertex < Graph::g->nb_vertices; ++vertex) {
            if (Graph::g->neighborhood[vertex].empty())
                continue;

            if (_colors[vertex] == color) {
                assert(_position[color][vertex] <
                       static_cast<int>(_colors_weights[color].size()));
            } else {
                assert(_position[color][vertex] >=
                       static_cast<int>(_colors_weights[color].size()));
            }

            if (vertex < static_cast<int>(_colors_weights[color].size())) {
                assert(_colors[_color_partition[color][vertex]] == color);
            } else {
                assert(_colors[_color_partition[color][vertex]] != color);
            }
        }
    }

    for (const auto &v : _unassigned) {
        (void)v;
        assert(_colors[v] == -1);
    }

    int unassigned = 0;
    for (int v = 0; v < Graph::g->nb_vertices; ++v) {
        if (Graph::g->neighborhood[v].empty()) {
            continue;
        }
        if (_colors[v] == -1) {
            ++unassigned;
        }
    }
    assert(unassigned == static_cast<int>(_unassigned.size()));

    for (const int &color : _empty_colors) {
        (void)color;
        assert(max_colors_weights[color] == 0);
        assert(_colors_weights[color].empty());
    }

    assert(score == _score);
    return true;
}

void Solution::reset_tabu() {
    tabu = std::vector<long>(Graph::g->nb_vertices);
}

void Solution::reset_edge_weights() {
    for (auto &edges : _edge_weights) {
        std::fill(edges.begin(), edges.end(), 0);
    }

    for (const auto &[v1, v2] : Graph::g->edges_list) {
        _edge_weights[v1][v2] = 1;
        _edge_weights[v2][v1] = 1;
    }
}

void Solution::increment_edge_weights() {
    int cpt = 0;
    for (const auto &[edge1, edge2] : _conflict_edges) {
        _edge_weights[edge1][edge2] += 1;
        _conflicts_colors[_colors[edge1]][edge2] += 1;
        cpt += 1;
    }
    _nb_conflicts += cpt / 2;
}

int Solution::get_delta_score(const int vertex, const int color) const {
    const int vertex_weight{Graph::g->weights[vertex]};
    int diff{0};
    // if the vertex already have a color
    if (_colors[vertex] != -1) {
        diff = get_delta_old_color(vertex);
    }
    // if the new color is empty
    if (color == -1 or static_cast<int>(_colors_weights.size()) <= color or
        _colors_weights[color].empty()) {
        return vertex_weight + diff;
    }
    // if the vertex is heavier than the heaviest of the new color class
    if (vertex_weight > _colors_weights[color].back()) {
        // the delta is the difference between the vertex weight and the heavier
        // vertex
        return vertex_weight - _colors_weights[color].back() + diff;
    }
    return diff;
}

[[nodiscard]] int Solution::get_delta_old_color(const int vertex) const {
    const int color = _colors[vertex];
    const int vertex_weight = Graph::g->weights[vertex];
    // if the vertex was the only one in the color
    if (_colors_weights[color].size() == 1) {
        return -vertex_weight;
    }
    // if the vertex is the heaviest one and the second heaviest is lighter
    if (vertex_weight == _colors_weights[color].back() and
        _colors_weights[color].end()[-2] < vertex_weight) {
        return _colors_weights[color].end()[-2] - vertex_weight;
    }
    // else
    return 0;
}

[[nodiscard]] int Solution::get_score_maybe_unassigned() const {
    return _unassigned.empty() ? _score : _unassigned_score;
}

[[nodiscard]] const std::vector<int> &Solution::unassigned() const {
    return _unassigned;
}

[[nodiscard]] int Solution::nb_free_colors(const int &vertex) const {
    return _nb_free_colors[vertex];
}

[[nodiscard]] int Solution::score() const {
    return _score;
}

[[nodiscard]] int Solution::unassigned_score() const {
    return _unassigned_score;
}

[[nodiscard]] int Solution::get_rank_placed_vertices() const {
    return _free_vertices.back();
}

[[nodiscard]] bool Solution::has_unassigned_vertices() const {
    return not _unassigned.empty();
}

void Solution::add_unassigned_vertex(const int &vertex) {
    _unassigned.push_back(vertex);
}

void Solution::remove_unassigned_vertex(const int &vertex) {
    _unassigned.erase(std::remove(_unassigned.begin(), _unassigned.end(), vertex));
}

[[nodiscard]] const std::vector<int> &Solution::colors() const {
    return _colors;
}

[[nodiscard]] long Solution::nb_non_empty_colors() const {
    return static_cast<long>(_non_empty_colors.size());
}

[[nodiscard]] const std::vector<int> &Solution::non_empty_colors() const {
    return _non_empty_colors;
}

[[nodiscard]] int Solution::nb_colors() const {
    return _nb_colors;
}

[[nodiscard]] const std::vector<int> &Solution::free_vertices() const {
    return _free_vertices;
}

[[nodiscard]] int Solution::conflicts_colors(const int &color, const int &vertex) const {
    return _conflicts_colors[color][vertex];
}

[[nodiscard]] int Solution::nb_conflicts() const {
    return _nb_conflicts;
}

[[nodiscard]] std::vector<std::tuple<int, int>> Solution::conflict_edges() const {
    return _conflict_edges;
}

[[nodiscard]] bool Solution::has_conflicts(const int vertex) const {
    return _conflicts_colors[_colors[vertex]][vertex] != 0;
}

[[nodiscard]] int Solution::get_delta_conflicts(const int vertex, const int color) const {
    return _conflicts_colors[color][vertex] - _conflicts_colors[_colors[vertex]][vertex];
}

void Solution::reduce_nb_colors(const int nb_total_color) {
    assert(nb_total_color < _nb_colors);
    std::vector<int> to_delete;
    for (int color{nb_total_color}; color < _nb_colors; ++color) {
        to_delete.insert(to_delete.end(),
                         _colors_vertices[color].begin(),
                         _colors_vertices[color].end());
    }
    for (const auto &vertex : to_delete) {
        delete_vertex_from_color(vertex);
    }
    std::stable_sort(to_delete.begin(), to_delete.end());
    for (const auto &vertex : to_delete) {
        int min_col{0};
        for (const auto &color : _non_empty_colors) {
            if (_conflicts_colors[color][vertex] < _conflicts_colors[min_col][vertex]) {
                min_col = color;
            }
        }
        add_vertex_to_color(vertex, min_col);
    }
}

[[nodiscard]] std::vector<int>
Solution::nb_vertices_per_color(const int nb_colors_max) const {
    std::vector<int> nb_colors_per_col(nb_colors_max, 0);
    for (size_t i{0}; i < _colors_vertices.size(); ++i) {
        nb_colors_per_col[i] = static_cast<int>(_colors_vertices[i].size());
    }
    return nb_colors_per_col;
}

[[nodiscard]] int Solution::color(const int &vertex) const {
    return _colors[vertex];
}

[[nodiscard]] const std::vector<int> &Solution::colors_vertices(const int &color) const {
    return _colors_vertices[color];
}

[[nodiscard]] const std::vector<std::vector<int>> &Solution::colors_weights() const {
    return _colors_weights;
}

[[nodiscard]] int Solution::max_weight(const int &color) const {
    return _colors_weights[color].back();
}

[[nodiscard]] bool Solution::is_color_empty(const int color) const {
    return _colors_vertices[color].empty();
}

[[nodiscard]] std::string Solution::line_csv() const {
    return fmt::format("{},{},{},{}",
                       _non_empty_colors.size(),
                       _nb_conflicts,
                       _score,
                       fmt::join(_colors, ":"));
}

[[nodiscard]] int distance_approximation(const Solution &sol1, const Solution &sol2) {
    const int max_k{std::max(sol1.nb_colors(), sol2.nb_colors()) + 1};
    std::vector<std::vector<int>> same_color(max_k, std::vector<int>(max_k, 0));
    std::vector<int> maxi(max_k, 0);
    std::vector<int> sigma(max_k, 0);
    const auto &col1{sol1.colors()};
    const auto &col2{sol2.colors()};
    for (int vertex{0}; vertex < Graph::Graph::g->nb_vertices; ++vertex) {
        ++same_color[col1[vertex]][col2[vertex]];
        if (same_color[col1[vertex]][col2[vertex]] > maxi[col1[vertex]]) {
            maxi[col1[vertex]] = same_color[col1[vertex]][col2[vertex]];
            sigma[col1[vertex]] = col2[vertex];
        }
    }
    int sum{0};
    for (int color{0}; color < max_k; ++color) {
        sum += same_color[color][sigma[color]];
    }
    return Graph::Graph::g->nb_vertices - sum;
}

[[nodiscard]] int distance(const Solution &sol1, const Solution &sol2) {
    // get the max number of colors
    const int max_k{std::max(sol1.nb_colors(), sol2.nb_colors()) + 1};

    // compute the number of same color in sol1 and sol2
    std::vector<std::vector<int>> same_color(max_k, std::vector<int>(max_k, 0));

    const auto &col1{sol1.colors()};
    const auto &col2{sol2.colors()};
    for (int vertex{0}; vertex < Graph::Graph::g->nb_vertices; ++vertex) {
        ++same_color[col1[vertex]][col2[vertex]];
    }

    // find the corresponding color of sol1 in sol2
    std::vector<int> corresponding_color(max_k, 0);

    int distance{0};

    for (int c{0}; c < max_k; ++c) {
        // find highest number of same color
        int max_val{-1};
        int max_c1{-1};
        int max_c2{-1};
        for (int c1{0}; c1 < max_k; ++c1) {
            const auto max_element{
                std::max_element(same_color[c1].begin(), same_color[c1].end())};
            const int max_val_tmp{*max_element};
            if (max_val_tmp > max_val) {
                max_val = max_val_tmp;
                max_c1 = c1;
                max_c2 =
                    static_cast<int>(std::distance(same_color[c1].begin(), max_element));
            }
        }
        // add the highest number to the corresponding color
        corresponding_color[max_c1] = max_c2;
        distance += max_val;

        // delete same color
        std::fill(same_color[max_c1].begin(), same_color[max_c1].end(), -1);
        for (int i{0}; i < max_k; ++i) {
            same_color[i][max_c2] = -1;
        }
    }

    return Graph::Graph::g->nb_vertices - distance;
}

[[nodiscard]] bool ok_distance(const Solution &current_solution,
                               const std::vector<Solution> &past_solutions,
                               const int distance_max) {
    return std::all_of(past_solutions.begin(),
                       past_solutions.end(),
                       [current_solution, distance_max](Solution sol) {
                           return distance_approximation(sol, current_solution) >
                                  distance_max;
                       });
}