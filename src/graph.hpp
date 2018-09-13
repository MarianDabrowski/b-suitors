#ifndef GRAPH_H
#define GRAPH_H

#include <utility>
#include <vector>
#include <set>
#include <stdio.h>
#include <fstream>
#include <sstream>
#include <tuple>
#include <unordered_map>
#include <iostream>
#include <queue>
#include <mutex>
#include <atomic>
#include <unordered_set>
#include "blimit.hpp"

using vertex_t = unsigned long;
using cost_t = unsigned int;
using element_t = std::pair<vertex_t, cost_t>;

extern unsigned int max_thread_allowed;

extern std::mutex insertion_mut;

extern std::vector<vertex_t> converted_values;

struct element_comparator {
    bool operator()(const element_t &u, const element_t &v) {
      return ((u.second < v.second) || (u.second == v.second &&
                                        converted_values[u.first] < converted_values[v.first]));
    }
};

extern element_comparator comparator;

struct suitors_collection {
    unsigned int current_size;
    unsigned int max_size;
    unsigned int total;
    std::set<element_t, element_comparator> collection;

    void insert(element_t &element);
    element_t last();

    suitors_collection() : current_size(0), max_size(0), total(0),
                           collection(std::set<element_t, element_comparator>(comparator)) {}

    suitors_collection(unsigned int upper_limit) : current_size(0), max_size(upper_limit), total(0),
                           collection(std::set<element_t, element_comparator>(comparator)) {}

    void update(unsigned int upper_limit);
};

struct vertex {
    std::atomic<int> num_of_connections;
    int iterator;
    unsigned int max_size;
    std::vector<element_t> neighbours;
    suitors_collection suitors;
    std::mutex mut;

    vertex(unsigned int upper_limit) {
        num_of_connections = 0;
        neighbours = std::vector<element_t>();
        suitors = suitors_collection();
        iterator = -1;
        max_size = upper_limit;
    }

    vertex(vertex &&other) {
        neighbours = std::move(other.neighbours);
        suitors = std::move(other.suitors);
        iterator = std::move(other.iterator);
        max_size = std::move(other.max_size);
        num_of_connections = other.num_of_connections.load();
    }
};

struct graph {
    std::vector<vertex> vertexes;
    unsigned int method;

    std::vector<std::tuple<vertex_t, vertex_t, cost_t >> _read_file(char *file_name);

    void fill_neigbours(char * file_name);

    void set_method(unsigned int b_method) { method = b_method;}

    void update();

    unsigned int get_result();

    element_t argmax(unsigned int u);
};

void b_suitor(graph& g);

#endif //GRAPH_H
