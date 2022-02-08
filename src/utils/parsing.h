#pragma once

#include <memory>
#include <string>

#include "../representation/Graph.h"
#include "../representation/Method.h"

/**
 * @brief parse the argument for the search
 *
 * @param argc : number of arguments given to main
 * @param argv : list of arguments
 * @return std::shared_ptr<Method>
 */
std::shared_ptr<Method> parse(int argc, const char **argv);

/**
 * Load graph previously reduced from a _r.col file from the
 * instances/wvcp_reduced/ directory
 *
 * @param instance_name : name of the instance (without the .col extension)
 * @param wvcp_problem : true if wvcp problem, false if gcp problem
 * @return : shared ptr to the Graph
 */
const std::unique_ptr<const Graph> load_graph(const std::string &instance_name,
                                              const bool wvcp_problem);
