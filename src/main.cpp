#include <csignal>

#include "utils/parsing.h"

#include "representation/Parameters.h"

void signal_handler(int signum) {
    fmt::print(stderr, "\nInterrupt signal ({}) received.\n", signum);
    Parameters::p->time_stop = std::chrono::high_resolution_clock::now();
}

int main(int argc, const char *argv[]) {
    // see src/utils/parsing.cpp for default parameters
    // Get the method
    auto method(parse(argc, argv));

    // Set the signal handler to stop the search
    signal(SIGTERM, signal_handler);
    signal(SIGINT, signal_handler);

    // Start the search
    method->run();
    Parameters::p->end_search();
}
