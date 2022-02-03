#pragma once

#include "../representation/Method.h"
#include "../representation/Solution.h"
#include "../representation/enum_types.h"

/** @brief Pointer to initialization function*/
typedef void (*init_ptr)(Solution &solution);
/** @brief Pointer to local search function*/
typedef void (*local_search_ptr)(Solution &, const bool);

/**
 * @brief Method for local search
 *
 */
class LocalSearch : public Method {

    /** @brief Best found solution*/
    Solution _best_solution;

    /** @brief Init function*/
    init_ptr _init_function;
    /** @brief Local search function*/
    local_search_ptr _local_search_function;

  public:
    explicit LocalSearch();

    ~LocalSearch() override = default;

    /**
     * @brief Run function for the method
     */
    void run() override;

    /**
     * @brief Return method header in csv format
     *
     * @return std::string method header in csv format
     */
    [[nodiscard]] const std::string header_csv() const override;

    /**
     * @brief Return method in csv format
     *
     * @return std::string method in csv format
     */
    [[nodiscard]] const std::string line_csv() const override;
};

/**
 * @brief Get the initialization
 *
 * @param initialization
 * @return init_ptr function greedy
 */
init_ptr get_initialization_fct(const Initialization &initialization);

/**
 * @brief Get the local search
 *
 * @param local_search
 * @return local_search_ptr function local search
 */
local_search_ptr get_local_search_fct(const Local_search &local_search);
