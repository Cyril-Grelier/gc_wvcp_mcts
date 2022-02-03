#include "Method.h"

#include "Solution.h"

void Method::signal_handler(int signum) {
    fmt::print(stderr, "\nInterrupt signal ({}) received.\n", signum);
    Parameters::p->time_stop = std::chrono::high_resolution_clock::now();
}
