#include<iostream>
#include<sstream>
#include<unordered_map>

#include<gtest/gtest.h>

#include "TestUtils.hpp"

#include "Cut.hpp"

using namespace cut;

// Run the algorithm by speifiying a tree via the standard input. 
// The input format is the following:
// In the first line there are 5 integers n, r, k, e_n, e_d. 
// n is the number of nodes in the tree, r is the id of the root in the tree and k is the number
// of parts in which the tree should be partitioned. e_n and e_d are the enumerator and the denominator
// of the approximation parameter epsilon respectively.
// n-1 lines follow which describe the edges of the tree. Each of these lines consists of 3 integers f, t, w.
// (f, t) is an edge in the tree and w is the corresponding weight of the edge.
TEST(Run, DISABLED_FromStdinVerbose) {
    testutils::AlgorithmParameters params;
    std::cin >> params;

    Tree tree = Tree::build_tree(params.tree, params.root_id);

    std::cerr << "digraph tree {\n";
    for (size_t lvl_idx = tree.levels.size() - 1; lvl_idx > 0; --lvl_idx) {
        for (size_t node_idx = 0; node_idx < tree.levels[lvl_idx].size(); ++node_idx) {
            Node& node = tree.levels[lvl_idx][node_idx];
            std::cerr << "\t" << tree.levels[lvl_idx - 1][node.parent_idx].id;
            std::cerr << " -> " << node.id;
            std::cerr << "[label=\"" << node.parent_edge_weight << "\"]\n";
        }
    }

    int32_t invis_node = -1;
    for (size_t lvl_idx = tree.levels.size() - 1; lvl_idx > 0; --lvl_idx) {
        std::vector<Node>& lvl = tree.levels[lvl_idx];
        std::stringstream node_ordering; 
        node_ordering << "{rank=same " << lvl[0].id;
        for (size_t node_idx = 1; node_idx < tree.levels[lvl_idx].size(); ++node_idx) {
            Node& node = lvl[node_idx];
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

    std::cerr << "comp_size_bounds:";
    for (auto& comp_size_bound : calculate_component_size_bounds(params.eps, params.node_cnt, params.part_cnt)) {
        std::cerr << " " << comp_size_bound;
    }
    std::cerr << "\n" << std::endl;

    auto signatures = tree.cut(params.eps, params.part_cnt);

    for (size_t lvl_idx = 0; lvl_idx < tree.levels.size(); ++lvl_idx) {
        for (size_t node_idx = 0; node_idx < tree.levels[lvl_idx].size(); ++node_idx) {
            std::cerr << "signatures of node " << tree.levels[lvl_idx][node_idx].id << ":\n";
            for (auto& sigs_with_size : signatures.signatures[lvl_idx][node_idx]) {
                std::cerr << " node_cnt " << sigs_with_size.first << ":\n";
                for (auto& sig : sigs_with_size.second) {
                    for (auto comp_size : sig.first) {
                        std::cerr << " " << comp_size;
                    }
                    std::cerr << " cut_cost: " << sig.second << "\n";
                }
            }
        }
    }
    std::cerr << std::endl;
}
