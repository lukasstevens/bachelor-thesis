#include<iostream>
#include<unordered_map>

#include<gtest/gtest.h>

#include "Partition.hpp"

using namespace part;

TEST(Run, DISABLED_FromStdinVerbose) {
    SizeType node_cnt;     
    Node::IdType root_id;
    SizeType part_cnt;     
    int32_t eps_num;
    int32_t eps_denom;
    std::cin >> node_cnt >> root_id >> part_cnt >> eps_num >> eps_denom;

    std::unordered_map<Node::IdType, std::unordered_map<Node::IdType, Node::EdgeWeightType>> tree_map;
    for (size_t edge_idx = 0; edge_idx < node_cnt - 1; ++edge_idx) {
        Node::IdType from; 
        Node::IdType to; 
        Node::EdgeWeightType weight;
        std::cin >> from >> to >> weight;
        tree_map[from][to] = weight;
    }

    Tree tree = Tree::build_tree(tree_map, root_id);

    std::cerr << "digraph tree {\n";
    for (size_t lvl_idx = tree.levels.size() - 1; lvl_idx > 0; --lvl_idx) {
        for (size_t node_idx = 0; node_idx < tree.levels[lvl_idx].size(); ++node_idx) {
            Node& node = tree.levels[lvl_idx][node_idx];
            std::cerr << "\t" << tree.levels[lvl_idx - 1][node.parent_idx].id;
            std::cerr << " -> " << node.id;
            std::cerr << "[label=\"" << node.parent_edge_weight << "\"]\n";
        }
    }
    std::cerr << "\n}\n" << std::endl;

    std::cerr << "comp_size_bounds:";
    for (auto& comp_size_bound : Tree::calculate_component_size_bounds(Tree::RationalType(eps_num, eps_denom), node_cnt, part_cnt)) {
        std::cerr << " " << comp_size_bound;
    }
    std::cerr << "\n" << std::endl;

    auto signatures = tree.partition(Tree::RationalType(eps_num, eps_denom), part_cnt);
}

