#include <algorithm>
#include <thread>
#include "graph.hpp"

std::vector<vertex_t> converted_values;

element_comparator comparator;

std::mutex insertion_mut;

std::vector<vertex_t> q;

std::unordered_set<vertex_t> r;

unsigned int max_thread_allowed;

element_t suitors_collection::last() {
    if (current_size < max_size) return std::make_pair((vertex_t) 0, (cost_t) 0);
    else return *collection.begin();
}

void suitors_collection::insert(element_t &element) {
    if (last().second == 0) {
        ++current_size;
        total += element.second;
        collection.insert(element);
    } else {
        element_t last_element = last();
        if (comparator(last_element, element)) {
            total -= last_element.second;
            total += element.second;
            collection.erase(last_element);
            collection.insert(element);
        }
    }
}

void suitors_collection::update(unsigned int upper_limit) {
    current_size = 0;
    max_size = upper_limit;
    total = 0;
    collection = std::set<element_t, element_comparator>(comparator);
}

std::vector<std::tuple<vertex_t, vertex_t, cost_t >> graph::_read_file(char *file_name) {
    std::ifstream my_file(file_name);
    std::string line;
    vertex_t v1, v2;
    cost_t cost;

    std::vector<std::tuple<vertex_t, vertex_t, cost_t>> output_vector;
    if (my_file.is_open()) {
        while (getline(my_file, line)) {

            std::stringstream stream(line);
            if (line[0] == '#') continue;
            stream >> v1 >> v2 >> cost;
            output_vector.emplace_back(std::make_tuple(v1, v2, cost));
        }
    }
    return output_vector;
}

void graph::fill_neigbours(char *file_name) {
    auto vector_of_triples = _read_file(file_name);

    unsigned int index = 0;
    vertex_t source, target;
    cost_t cost;
    std::unordered_map<vertex_t, vertex_t> map;
    for (auto &element :vector_of_triples) {
        if (map.insert(std::make_pair(std::get<0>(element), (vertex_t) index)).second) {
            converted_values.push_back(std::get<0>(element));
            ++index;
        }
        if (map.insert(std::make_pair(std::get<1>(element), (vertex_t) index)).second) {
            converted_values.push_back(std::get<1>(element));
            ++index;
        }
    }

    for (unsigned int i = 0; i < converted_values.size(); ++i) vertexes.emplace_back(vertex(0));

    for (auto &element :vector_of_triples) {
        source = map.find(std::get<0>(element))->second;
        target = map.find(std::get<1>(element))->second;
        cost = std::get<2>(element);
        vertexes[source].neighbours.emplace_back(std::make_pair(target, cost));
        vertexes[target].neighbours.emplace_back(std::make_pair(source, cost));
    }

    for (vertex& v : vertexes) {
        std::sort(v.neighbours.begin(), v.neighbours.end(), comparator);
    }
}

void graph::update() {
    unsigned int upper_limit;

    for (unsigned int i = 0; i < vertexes.size(); ++i) {
        upper_limit = bvalue(method, converted_values[i]);
        vertexes[i].max_size = upper_limit;
        vertexes[i].suitors.update(upper_limit);
        vertexes[i].iterator = (int)vertexes[i].neighbours.size() - 1;
        vertexes[i].num_of_connections = 0;
    }
}

unsigned int graph::get_result() {
    unsigned int sum = 0;

    for(unsigned int i = 0; i < vertexes.size(); ++i) {
        sum += vertexes[i].suitors.total;
    }
    return sum / 2;
}

element_t graph::argmax(unsigned int u) {
    if(vertexes[u].iterator >= 0) {
        element_t p = vertexes[u].neighbours[vertexes[u].iterator];
        --vertexes[u].iterator;
        vertexes[p.first].mut.lock();
        if (0 < vertexes[p.first].max_size &&
            comparator(vertexes[p.first].suitors.last(), std::make_pair(u, p.second))) {
            return p;
        }
        else {
            vertexes[p.first].mut.unlock();
            return argmax(u);
        }
    }
    return std::make_pair((vertex_t) 0, (cost_t) 0);
}


void run_thread(graph& g, unsigned int initial, unsigned step) {

    vertex_t u, x, y;
    for(unsigned int i = 0; initial + i * step < q.size(); ++i) {
        u = q[initial + i * step];
        while (g.vertexes[u].num_of_connections < g.vertexes[u].max_size) {

            element_t element = g.argmax(u);
            if (element.second == 0) break;
            else {
                x = element.first;
                element_t y_pair = g.vertexes[x].suitors.last();
                element_t u_pair = std::make_pair(u, element.second);

                g.vertexes[x].suitors.insert(u_pair);
                g.vertexes[u].num_of_connections++;

                if (y_pair.second != 0 && comparator(y_pair, u_pair)) {
                    y = y_pair.first;
                    g.vertexes[y].num_of_connections--;
                    insertion_mut.lock();
                    r.insert(y);
                    insertion_mut.unlock();
                }
                g.vertexes[x].mut.unlock();
            }
        }
    }
}


void b_suitor(graph &g) {

    std::vector<std::thread> threads;
    unsigned int step = 1;
    for (unsigned int i = 0; i < converted_values.size(); ++i) q.push_back(i);

    while (!q.empty()) {

        if (max_thread_allowed == 1) run_thread(g, 0, step);
        else {
            step = max_thread_allowed - 1;
            for (unsigned int i = 0; i < max_thread_allowed - 1; ++i) {
                threads.emplace_back(std::thread{[&g, i, step] {run_thread(g, i, step);}});
            }

            for (auto& t : threads) {
                t.join();
            }

            threads.clear();
        }

        q.clear();
        for (vertex_t v : r) {
            q.push_back(v);
        }
        r.clear();
    }
}