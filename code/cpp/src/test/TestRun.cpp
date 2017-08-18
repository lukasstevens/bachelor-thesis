#include<cstdint>
#include<iostream>
#include<sstream>
#include<unordered_map>

#include<gtest/gtest.h>

#include "Partition.hpp"

using namespace cut;

// Run the algorithm by speifiying a tree via the standard input. 
// The input format is the following:
// In the first line there are 2 integers n, r. 
// n is the number of nodes in the tree, r is the id of the root.
// n-1 lines follow which describe the edges of the tree. Each of these lines consists of 3 integers f, t, w.
// (f, t) is an edge in the tree and w is the corresponding weight of the edge.
// A single line follows containing integers k, e_n, e_d where k is the number of parts into which
// the tree shall be partitioned and e_n/e_d describes the approximation parameter epsilon.
TEST(Run, DISABLED_FromStdinVerbose) {
    using Id = int32_t;
    using EdgeWeight = int32_t;
    using Tree = Tree<int32_t, int32_t>;

    Tree tree;
    std::cin >> tree;

    std::cerr << tree.as_graphviz();

    Tree::SizeType part_cnt;
    int64_t eps_num;
    int64_t eps_denom;
    std::cin >> part_cnt >> eps_num >> eps_denom;
    RationalType eps(eps_num, eps_denom);

    std::cerr << "comp_size_bounds:";
    for (auto& comp_size_bound : calculate_upper_component_size_bounds(eps, tree.tree_sizes[0][0], part_cnt)) {
        std::cerr << " " << comp_size_bound;
    }
    std::cerr << "\n" << std::endl;

    auto signatures = tree.cut(eps, part_cnt);

    std::cerr << signatures << std::endl;

    std::vector<std::set<Tree::SizeType>> partitioning;
    Tree::Signature signature;
    EdgeWeight cut_cost;
    std::tie(partitioning, signature, cut_cost) = part::calculate_best_packing(signatures);

    ASSERT_LE(partitioning.size(), static_cast<size_t>(part_cnt));
    size_t node_cnt_in_partitioning = 0;
    for (auto const& part : partitioning) {
        node_cnt_in_partitioning += part.size();
        ASSERT_LT(part.size(), signatures.upper_comp_size_bounds.back());
    }
    ASSERT_EQ(tree.tree_sizes[0][0], node_cnt_in_partitioning);

    std::cerr << "partitioning:" << std::endl;
    for (auto const& part : partitioning) {
        for (auto const node : part) {
            std::cerr << " " << node;
        }
        std::cerr << std::endl;
    }

    std::cerr << "signature:";
    for (auto const comp_cnt : signature) {
        std::cerr << " " << comp_cnt;
    }
    std::cerr << std::endl;
    std::cerr << "cut_cost: " << cut_cost << std::endl;


    std::cerr << "digraph tree {\n\tedge[dir=none]\n";

    auto get_part_idx = [partitioning](Id node_id){
        size_t idx = 0;
        for (auto const& part : partitioning) {
            if (part.find(node_id) != part.end()) {
                return idx;
            }
            idx += 1;
        }
        return static_cast<size_t>(-1);
    };

    for (size_t idx = 0; idx < partitioning.size(); ++idx) {
        std::cerr << "\tsubgraph part" << idx << " {\n";
        std::cerr << "\t\tnode[style=filled, color=\"/spectral9/" << (idx + 1) << "\"]\n";
        for (auto node_id : partitioning[idx]) {
            std::cerr << "\t\t" << node_id << "\n"; 
        }
        std::cerr << "\t}\n";
    }

    for (size_t lvl_idx = tree.levels.size() - 1; lvl_idx > 0; --lvl_idx) {
        for (size_t node_idx = 0; node_idx < tree.levels[lvl_idx].size(); ++node_idx) {
            Tree::Node& node = tree.levels[lvl_idx][node_idx];
            Tree::Node& parent = tree.levels[lvl_idx - 1][node.parent_idx];
            std::cerr << "\t" << parent.id;
            std::cerr << " -> " << node.id;
            std::cerr << "[label=\"" << node.parent_edge_weight << "\"";
            if (get_part_idx(node.id) != get_part_idx(parent.id)) {
                std::cerr << ", style=dashed";
            }
            std::cerr << "]\n";
        }
    }

    int32_t invis_node = -1;
    for (size_t lvl_idx = tree.levels.size() - 1; lvl_idx > 0; --lvl_idx) {
        std::vector<Tree::Node>& lvl = tree.levels[lvl_idx];
        std::stringstream node_ordering; 
        node_ordering << "{rank=same " << lvl[0].id;
        for (size_t node_idx = 1; node_idx < tree.levels[lvl_idx].size(); ++node_idx) {
            Tree::Node& node = lvl[node_idx];
            std::cerr << "\t" << invis_node << "[label=\"\", width=0.1, style=invis]\n";
            std::cerr << "\t" << tree.levels[lvl_idx - 1][node.parent_idx].id;
            std::cerr << " -> " << invis_node << "[style=invis]\n";
            node_ordering << " -> " << invis_node << " -> " << node.id;
            --invis_node;
        }
        if (lvl.size() > 1) {
            node_ordering << "[style=invis]";
        }
        node_ordering << "}\n";
        std::cerr << node_ordering.str();
    }

    std::cerr << "\n}\n" << std::endl;
}

TEST(Run, DISABLED_FromStdinCutting) {
    using Id = int32_t;
    using EdgeWeight = int32_t;
    using Tree = Tree<Id, EdgeWeight>;

    Tree tree;
    std::cin >> tree;

    Tree::SizeType part_cnt;
    int64_t eps_num;
    int64_t eps_denom;
    std::cin >> part_cnt >> eps_num >> eps_denom;
    RationalType eps(eps_num, eps_denom);

    auto signatures = tree.cut(eps, part_cnt);

    std::cerr << signatures << std::endl;
}
