#include<fstream>
#include<set>
#include<string>

#include<gtest/gtest.h>

#include "Cut.cpp"
#include "TestUtils.hpp"

TEST(CutEdgesForSignature, Tree2) {
    testutils::AlgorithmParameters params;    
    std::string resource_filename("resources/2.in");
    std::ifstream stream(resource_filename);
    ASSERT_TRUE(stream.good()) << "Failed to open file " << resource_filename << ".";
    stream >> params;

    cut::Tree tree = cut::Tree::build_tree(params.tree, params.root_id);

    auto signatures = tree.cut(params.eps, params.part_cnt);

    auto comp_size_bounds = cut::calculate_component_size_bounds(params.eps, params.node_cnt, params.part_cnt);
    cut::Signature all_edges_cut(comp_size_bounds.size());
    size_t comp_size_one_idx = 0;
    while (comp_size_bounds[comp_size_one_idx] <= 1) { 
        ++comp_size_one_idx;
    }
    all_edges_cut[comp_size_one_idx] = params.node_cnt;

    auto cut_edges = signatures.cut_edges_for_signature(all_edges_cut);

    ASSERT_EQ(cut_edges.size(), 4);

    std::set<std::pair<Node::IdType, Node::IdType>> should_cut_edges;
    should_cut_edges.insert(std::make_pair(2, 1));
    should_cut_edges.insert(std::make_pair(3, 1));
    should_cut_edges.insert(std::make_pair(4, 2));
    should_cut_edges.insert(std::make_pair(5, 2));

    ASSERT_EQ(should_cut_edges, cut_edges);

}
