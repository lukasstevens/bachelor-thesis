#include<iostream>
#include<sstream>
#include<stdexcept>
#include<string>

#include<gtest/gtest.h>

#include "Graph.hpp"
#include "GraphIo.hpp"

TEST(Graph, EmptyInput) {
    graph::Graph<> graph;
    std::stringstream graph_stream("");
    ASSERT_THROW(graph_stream >> graph, std::invalid_argument);
}

TEST(Graph, NoNodeWeight) {
    graph::Graph<> graph;
    std::stringstream graph_stream("1 0 010\n\n");
    ASSERT_THROW(graph_stream >> graph, std::invalid_argument);
}

TEST(Graph, NoEdgeWeight) {
    graph::Graph<> graph;
    std::stringstream graph_stream("2 0 001\n1\n0\n");
    ASSERT_THROW(graph_stream >> graph, std::invalid_argument);
}

TEST(Graph, NodeSizes) {
    graph::Graph<> graph;
    std::stringstream graph_stream("1 0 100\n");
    ASSERT_THROW(graph_stream >> graph, std::invalid_argument);
}

TEST(Graph, MultipleNodeWeights) {
    graph::Graph<> graph;
    std::stringstream graph_stream("1 0 010 2\n");
    ASSERT_THROW(graph_stream >> graph, std::invalid_argument);
}

TEST(Graph, OneNodeMinimal) {
    graph::Graph<> graph;
    std::stringstream graph_stream("1 0\n");
    graph_stream >> graph;
    ASSERT_EQ(graph.node_cnt(), 1);
    ASSERT_EQ(graph.adj_nodes(0).size(), 0);
    ASSERT_EQ(graph.node_weight(0), 1);
}

TEST(Graph, OneNode) {
    graph::Graph<> graph;
    std::stringstream graph_stream("1 0 010\n");
    graph_stream >> graph;
    ASSERT_EQ(graph.node_cnt(), 1);
    ASSERT_EQ(graph.adj_nodes(0).size(), 0);
    ASSERT_EQ(graph.node_weight(0), 1);

    graph_stream.str("1 0 010\n2\n");
    graph_stream.clear();
    graph_stream >> graph;
    ASSERT_EQ(graph.node_cnt(), 1);
    ASSERT_EQ(graph.adj_nodes(0).size(), 0);
    ASSERT_EQ(graph.node_weight(0), 2);
}

TEST(Graph, TwoNodes) {
    graph::Graph<> graph;
    std::stringstream graph_stream("2 1 001\n1 2\n0 2\n"); 
    graph_stream >> graph;
    ASSERT_EQ(graph.node_cnt(), 2);
    ASSERT_EQ(graph.adj_nodes(0), std::vector<int>({1}));
    ASSERT_EQ(graph.adj_nodes(1), std::vector<int>({0}));
    using Edge = std::pair<int, int>;
    ASSERT_EQ(graph.inc_edges(0), std::vector<Edge>({std::make_pair(1, 2)}));
    ASSERT_EQ(graph.inc_edges(1), std::vector<Edge>({std::make_pair(0, 2)}));
    ASSERT_EQ(graph.edge_weight(0, 1), 2);
    ASSERT_EQ(graph.edge_weight(1, 0), 2);
    ASSERT_EQ(graph.node_weight(0), 1);
    ASSERT_EQ(graph.node_weight(1), 1);
}

