#include<algorithm>
#include<iostream>
#include<sstream>
#include<stdexcept>
#include<string>

#include<gtest/gtest.h>
#include<metis.h>

#include "Graph.hpp"
#include "GraphIo.hpp"
#include "GraphUtils.hpp"
#include "GraphGen.hpp"
#include "GMPUtils.hpp"

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

TEST(Graph, ContractEdges) {
    graph::Graph<> graph;
    std::stringstream graph_stream("3 3 011\n1 1 1 2 2\n2 0 1 2 3\n2 0 2 1 3\n"); 
    graph_stream >> graph;
    graph::Graph<>::Matching matching({std::make_pair(0, 1)});
    graph::Graph<> contracted_graph = graph.contract_edges(matching);
    ASSERT_EQ(contracted_graph.node_cnt(), 2);
    ASSERT_EQ(contracted_graph.edge_weight(0, 1), 5);
    ASSERT_EQ(contracted_graph.inc_edges(0).size(), 1);
    ASSERT_EQ(contracted_graph.node_repr(0), graph::Graph<>::NodeSet({0, 1}));
    ASSERT_EQ(contracted_graph.node_repr(1), graph::Graph<>::NodeSet({2}));
}

TEST(Graph, HeavyEdgeMatching) {
    graph::Graph<> graph;
    std::stringstream graph_stream("5 6 001\n1 1\n0 1 2 1 3 1\n1 1 3 2 4 1\n1 1 2 2 4 2\n2 1 3 2\n"); 
    graph_stream >> graph;
    auto matching = graph::heavy_edge_matching(graph);
    ASSERT_EQ(matching.size(), 2);
    ASSERT_TRUE(std::find(matching.begin(), matching.end(), std::make_pair(0, 1)) != matching.end()
            || std::find(matching.begin(), matching.end(), std::make_pair(1, 0)) != matching.end());
    ASSERT_TRUE(std::find(matching.begin(), matching.end(), std::make_pair(3, 4)) != matching.end()
            || std::find(matching.begin(), matching.end(), std::make_pair(4, 3)) != matching.end());
}

// Metis sometimes violates the constraints because of rounding errors. Therefore this function checks
// if metis violates the size constraint of the partitions.
bool violates_max_part_size(std::vector<int> partition, int kparts, graph::Rational imbalance) {
    int max_part_size = gmputils::floor_to_int<int>((graph::Rational(1) + imbalance) *
        gmputils::ceil_to_int<int>(graph::Rational(partition.size(), kparts)));

    std::vector<int> part_size(static_cast<size_t>(kparts));
    for (auto const in_part : partition) {
        part_size.at(static_cast<size_t>(in_part)) += 1;
    }
    
    return std::any_of(part_size.cbegin(), part_size.cend(),
            [max_part_size](int const part_size){ return part_size > max_part_size; });
}

TEST(Graph, Metis) {
    graphgen::IGraphGen<>* graph_gen_rand = new graphgen::TreeRandAttach<>(30);
    graphgen::IGraphGen<>* graph_gen_pref = new graphgen::TreePrefAttach<>(30);
    graph::Graph<> graph;
    graph::Rational imbalance(1,3);

    for (size_t seed = 0; seed < 30; ++seed) {
        graph = (*graph_gen_rand)(seed);
        auto res = graph.partition(3, imbalance);
        auto metis_rec_res = graph.partition_metis_recursive(3, imbalance);
        auto metis_kway_res = graph.partition_metis_kway(3, imbalance);
        if (!violates_max_part_size(metis_rec_res.second, 3, imbalance)) {
            EXPECT_LE(res.first, metis_rec_res.first);
        }
        if (!violates_max_part_size(metis_kway_res.second, 3, imbalance)) {
            EXPECT_LE(res.first, metis_kway_res.first);
        }

        graph = (*graph_gen_pref)(seed);
        res = graph.partition(3, imbalance);
        metis_rec_res = graph.partition_metis_recursive(3, imbalance);
        metis_kway_res = graph.partition_metis_kway(3, imbalance);
        if (!violates_max_part_size(metis_rec_res.second, 3, imbalance)) {
            EXPECT_LE(res.first, metis_rec_res.first);
        }
        if (!violates_max_part_size(metis_kway_res.second, 3, imbalance)) {
            EXPECT_LE(res.first, metis_kway_res.first);
        }
    }

    delete graph_gen_rand;
    delete graph_gen_pref;
}

TEST(Graph, KaHIP) {
    graphgen::IGraphGen<>* graph_gen_rand = new graphgen::TreeRandAttach<>(30);
    graphgen::IGraphGen<>* graph_gen_pref = new graphgen::TreePrefAttach<>(30);
    graph::Graph<> graph;
    graph::Rational imbalance(1,3);
    for (size_t seed = 0; seed < 30; ++seed) {
        graph = (*graph_gen_rand)(seed);
        auto res = graph.partition(3, imbalance);
        auto kahip_res = graph.partition_kaffpa(3, imbalance);
        EXPECT_LE(res.first, kahip_res.first);
        graph = (*graph_gen_pref)(seed);
        res = graph.partition(3, imbalance);
        kahip_res = graph.partition_kaffpa(3, imbalance);
        EXPECT_LE(res.first, kahip_res.first);
    }

    delete graph_gen_rand;
    delete graph_gen_pref;
}

TEST(Graph, ImpossibleNodeWeights) {
    graph::Graph<> graph;
    std::istringstream graph_stream("3 2 010\n2 1\n2 0 2\n2 1\n");
    graph_stream >> graph;
    ASSERT_THROW(
        auto part = graph.partition(2, graph::Rational(1,4)),
        part::PartitionException
    );
}

TEST(GenGraph, TreeFat) {
    graphgen::IGraphGen<>* graph_gen =
        new graphgen::TreeFat<>(100, std::make_pair(2, 6));
    graph::Graph<> graph = (*graph_gen)(0);
    delete graph_gen;
}
