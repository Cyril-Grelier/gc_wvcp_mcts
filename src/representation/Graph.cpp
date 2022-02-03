#include "Graph.h"

std::unique_ptr<const Graph> Graph::g = nullptr;

void Graph::init_graph(std::unique_ptr<const Graph> graph_) {
    Graph::g = std::move(graph_);
}

Graph::Graph(const std::string &name_,
             const int &nb_vertices_,
             const int &nb_edges_,
             const std::vector<std::pair<int, int>> &edges_list_,
             const std::vector<std::vector<bool>> &adjacency_matrix_,
             const std::vector<std::vector<int>> &neighborhood_,
             const std::vector<int> &degrees_,
             const std::vector<int> &weights_)
    : name(name_),
      nb_vertices(nb_vertices_),
      nb_edges(nb_edges_),
      edges_list(edges_list_),
      adjacency_matrix(adjacency_matrix_),
      neighborhood(neighborhood_),
      degrees(degrees_),
      weights(weights_) {
}
