cmake_minimum_required(VERSION 3.14 FATAL_ERROR)
project(gc_wvcp)

set(CMAKE_CXX_STANDARD 17)

# -Wsign-conversion -Weffc++
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_ARCH_FLAGS} -O0 -g -isystem dir -Wall -Wextra -Wshadow -Wnon-virtual-dtor -Wold-style-cast -Wcast-align -Wunused -Woverloaded-virtual -Wpedantic -Wconversion -Wmisleading-indentation -Wduplicated-cond -Wduplicated-branches -Wlogical-op -Wnull-dereference -Wuseless-cast -Wdouble-promotion -Wformat=2")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_ARCH_FLAGS} -O3 -DNDEBUG -Wall -s -isystem dir -Wall -Wextra -Wshadow -Wnon-virtual-dtor -Wold-style-cast -Wcast-align -Wunused -Woverloaded-virtual -Wpedantic -Wconversion -Wmisleading-indentation -Wduplicated-cond -Wduplicated-branches -Wlogical-op -Wnull-dereference -Wuseless-cast -Wdouble-promotion -Wformat=2")

add_executable(${CMAKE_PROJECT_NAME}
    src/main.cpp

    # utils
    src/utils/random_generator.cpp src/utils/random_generator.h
    src/utils/utils.cpp src/utils/utils.h

    # representation
    src/representation/enum_types.cpp src/representation/enum_types.h
    src/representation/Graph.cpp src/representation/Graph.h
    src/representation/Method.h
    src/representation/Node.cpp src/representation/Node.h
    src/representation/Parameters.cpp src/representation/Parameters.h
    src/representation/ProxiSolutionILSTS.cpp src/representation/ProxiSolutionILSTS.h
    src/representation/ProxiSolutionRedLS.cpp src/representation/ProxiSolutionRedLS.h
    src/representation/Solution.cpp src/representation/Solution.h

    # methods
    src/methods/afisa.cpp src/methods/afisa.h
    src/methods/afisa_original.cpp src/methods/afisa_original.h
    src/methods/greedy.cpp src/methods/greedy.h
    src/methods/hill_climbing.cpp src/methods/hill_climbing.h
    src/methods/ilsts.cpp src/methods/ilsts.h
    src/methods/LocalSearch.cpp src/methods/LocalSearch.h
    src/methods/MCTS.h src/methods/MCTS.cpp
    src/methods/redls.cpp src/methods/redls.h
    src/methods/redls_freeze.cpp src/methods/redls_freeze.h
    src/methods/tabu_col.cpp src/methods/tabu_col.h
    src/methods/tabu_weight.cpp src/methods/tabu_weight.h
)

set_property(TARGET ${CMAKE_PROJECT_NAME} PROPERTY CXX_STANDARD 17)

# add dependencies
set(CPM_DOWNLOAD_VERSION 0.32.0)

if(CPM_SOURCE_CACHE)
    set(CPM_DOWNLOAD_LOCATION "${CPM_SOURCE_CACHE}/cpm/CPM_${CPM_DOWNLOAD_VERSION}.cmake")
elseif(DEFINED ENV{CPM_SOURCE_CACHE})
    set(CPM_DOWNLOAD_LOCATION "$ENV{CPM_SOURCE_CACHE}/cpm/CPM_${CPM_DOWNLOAD_VERSION}.cmake")
else()
    set(CPM_DOWNLOAD_LOCATION "${CMAKE_BINARY_DIR}/cmake/CPM_${CPM_DOWNLOAD_VERSION}.cmake")
endif()

if(NOT(EXISTS ${CPM_DOWNLOAD_LOCATION}))
    message(STATUS "Downloading CPM.cmake to ${CPM_DOWNLOAD_LOCATION}")
    file(DOWNLOAD
        https://github.com/TheLartians/CPM.cmake/releases/download/v${CPM_DOWNLOAD_VERSION}/CPM.cmake
        ${CPM_DOWNLOAD_LOCATION}
    )
endif()

include(${CPM_DOWNLOAD_LOCATION})

CPMAddPackage(
    GITHUB_REPOSITORY jarro2783/cxxopts
    VERSION 2.2.1
    OPTIONS "CXXOPTS_BUILD_EXAMPLES NO" "CXXOPTS_BUILD_TESTS NO" "CXXOPTS_ENABLE_INSTALL YES"
)

CPMAddPackage(
    NAME fmt
    GIT_TAG 8.0.0
    GITHUB_REPOSITORY fmtlib/fmt
)

# link dependencies
target_link_libraries(${CMAKE_PROJECT_NAME} fmt cxxopts)