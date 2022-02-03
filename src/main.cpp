#include <csignal>

#include "utils/parsing.h"

#include "representation/Parameters.h"

int main(int argc, const char *argv[]) {
    // see top of file src/representation/Parameters.cpp for default parameters
    // Get the method
    auto method(parse(argc, argv));

    // Set the signal handler to stop the search
    signal(SIGTERM, method->signal_handler);
    signal(SIGINT, method->signal_handler);

    // Start the search
    method->run();
    Parameters::p->end_search();
}
