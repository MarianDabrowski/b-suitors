#include "blimit.hpp"
#include <iostream>
#include <thread>
#include "graph.hpp"


int main(int argc, char *argv[]) {
    std::ios_base::sync_with_stdio(0);
    if (argc != 4) {
        std::cerr << "usage: " << argv[0] << " thread-count inputfile b-limit" << std::endl;
        return 1;
    }

    int thread_count = std::stoi(argv[1]);
    int b_limit = std::stoi(argv[3]);
    char *input_filename{argv[2]};
    max_thread_allowed = thread_count;

    graph g = graph();
    g.fill_neigbours(input_filename);
    
    for (int b_method = 0; b_method < b_limit + 1; b_method++) {

        g.set_method(b_method);
        g.update();

        b_suitor(g);

        std::cout << g.get_result() << std::endl;
    }

}

