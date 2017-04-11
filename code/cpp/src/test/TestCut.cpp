#include<fstream>
#include<set>
#include<string>

#include<gtest/gtest.h>

#include "Cut.cpp"
#include "TestUtils.hpp"

TEST(Cut, Tree2) {
    testutils::AlgorithmParameters params;    
    std::string resource_filename("resources/2.in");
    std::ifstream stream(resource_filename);
    ASSERT_TRUE(stream.good()) << "Failed to open file " << resource_filename << ".";
    stream >> params;

    cut::Tree tree = cut::Tree::build_tree(params.tree, params.root_id);

    auto comp_size_bounds = cut::calculate_upper_component_size_bounds(params.eps, params.node_cnt, params.part_cnt);
    auto signatures = tree.cut(params.eps, params.part_cnt);
    auto& root_sigs = signatures.signatures[0][0].at(params.node_cnt);

    ASSERT_EQ(comp_size_bounds, std::vector<SizeType>({ 1, 2, 2, 3, 4, 4 }));

    std::vector<std::valarray<SizeType>> poss_signatures({
            {0, 5, 0, 0, 0, 0},
            {0, 2, 0, 0, 1, 0},
            {0, 1, 0, 2, 0, 0},
            {0, 0, 0, 1, 1, 0},
            {0, 3, 0, 1, 0, 0}
            });

    auto mk_p = [](Node::IdType one, Node::IdType other){ 
        return std::make_pair(one, other);
    };

    std::vector<std::set<std::pair<Node::IdType, Node::IdType>>> cut_edges_for_sigs({
            {mk_p(2,1), mk_p(3,1), mk_p(4,2), mk_p(5,2)},
            {mk_p(2,1), mk_p(3,1)},
            {mk_p(2,1), mk_p(5,2)},
            {mk_p(2,1)},
            {mk_p(2,1), mk_p(3,1), mk_p(5,2)}
            });

    std::vector<Node::EdgeWeightType> costs_for_sigs({
            13, 4, 5, 1, 8
            });

    for (size_t sig_idx = 0; sig_idx < poss_signatures.size(); ++sig_idx) {
        auto& poss_sig = poss_signatures[sig_idx];
        ASSERT_TRUE(root_sigs.find(poss_sig) != root_sigs.end());
        ASSERT_EQ(signatures.cut_edges_for_signature(poss_sig), cut_edges_for_sigs.at(sig_idx));
        ASSERT_EQ(root_sigs.at(poss_sig), costs_for_sigs[sig_idx]);
    }


    std::vector<std::vector<std::set<Node::IdType>>> components_for_signatures({
            {{1}, {2}, {3}, {4}, {5}},
            {{1}, {2, 4, 5}, {3}},
            {{1, 3}, {2, 4}, {5}},
            {{1, 3}, {2, 4, 5}},
            {{1}, {2, 4}, {3}, {5}}
            });
    for (size_t comps_idx = 0; comps_idx < components_for_signatures.size(); ++comps_idx) {
        auto is_comps = signatures.components_for_cut_edges(cut_edges_for_sigs[comps_idx]);
        for (auto& comp : components_for_signatures[comps_idx]) {
            ASSERT_NE(std::find(is_comps.begin(), is_comps.end(), comp), is_comps.end());
        }
    }
}
