#include<iostream>

#include "GraphIo.hpp"

int main(int argc, char** args) {
    graph::Graph<> graph;
    graphio::ReadSnapFormat<> read(graph);
    std::cin >> read;
    std::cout << graph;
}

