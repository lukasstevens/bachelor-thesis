#include<cmath>
#include<list>
#include<stdexcept>
#include<unordered_set>

#include "Cut.hpp"

namespace cut {

    using EdgeWeightType = Node::EdgeWeightType;
    using IdType = Node::IdType;

    Node::Node(IdType id, EdgeWeightType parent_edge_weight, 
            size_t parent_idx, std::pair<size_t, size_t> children_idx_range) : 
        id(id), parent_edge_weight(parent_edge_weight), parent_idx(parent_idx), children_idx_range(children_idx_range) {}


    Tree Tree::build_tree(std::unordered_map<IdType, std::unordered_map<IdType, EdgeWeightType>> const& tree_map, IdType root_id) {
        Tree tree;
        // Use a struct to represent an incomplete node since the child_idx_range is not known.
        struct NodeStub {
            IdType const id; 
            EdgeWeightType const parent_edge_weight;
            size_t const parent_idx;
            bool const has_left_sibling;
            size_t const level;

            NodeStub(IdType id, EdgeWeightType p_e_w, size_t p_i, bool h_l_s, size_t lvl) :
                id(id), parent_edge_weight(p_e_w), parent_idx(p_i), has_left_sibling(h_l_s), level(lvl) {}
        };

        // Do a BFS to build the tree.
        std::list<NodeStub> queue;
        queue.push_back(NodeStub(root_id, 0, 0, false, 0));
        size_t next_child_idx = 0;
        while(!queue.empty()) {
            NodeStub curr_node = queue.front();
            queue.pop_front();

            // Add a new level to tree if the BFS reaches a new level and reset the child_idx.
            if (curr_node.level == tree.levels.size()) {
                tree.levels.push_back(std::vector<Node>());
                tree.has_left_sibling.push_back(std::vector<bool>());
                next_child_idx = 0;
            }

            // Save the index of the first child.
            size_t old_next_child_idx = next_child_idx;
            if (tree_map.find(curr_node.id) != tree_map.cend()) {
                for (auto const neighbor : tree_map.at(curr_node.id)) {
                    // Check if neighbor is the parent.
                    if (curr_node.level == 0 || neighbor.first != tree.levels[curr_node.level - 1][curr_node.parent_idx].id) {
                        bool has_left_sibling = !(old_next_child_idx == next_child_idx);
                        queue.emplace_back(neighbor.first, neighbor.second, tree.levels[curr_node.level].size(), has_left_sibling, curr_node.level + 1); 
                        ++next_child_idx;
                    }
                }
            }

            tree.levels[curr_node.level].emplace_back(curr_node.id, curr_node.parent_edge_weight, 
                    curr_node.parent_idx, std::make_pair(old_next_child_idx, next_child_idx));
            tree.has_left_sibling[curr_node.level].push_back(curr_node.has_left_sibling);

        }


        // Calculate the sizes of the subtrees rooted at each vertex and store them in a two-dimensional vector.
        for (auto& lvl : tree.levels) {
            tree.tree_sizes.emplace_back(lvl.size(), 1);
        }
        for (size_t lvl_idx = tree.levels.size() - 2; lvl_idx < tree.levels.size(); --lvl_idx) {
            size_t node_idx = 0;
            for (auto& node : tree.levels[lvl_idx]) {
                for (size_t child_idx = node.children_idx_range.first; child_idx < node.children_idx_range.second; ++child_idx) {
                    tree.tree_sizes[lvl_idx][node_idx] += tree.tree_sizes[lvl_idx + 1][child_idx];
                }
                ++node_idx;
            }
        }

        return tree;
    }

    Tree Tree::build_tree(std::unordered_map<IdType, std::unordered_map<IdType, EdgeWeightType>> const& tree_map) {
        return build_tree(tree_map, tree_map.begin()->first);
    }

    std::vector<SizeType> Tree::calculate_component_size_bounds(Tree::RationalType eps, SizeType node_cnt, SizeType part_cnt) {
        using Rational = Tree::RationalType;

        // Calculate the sizes of the components in a signature according to the paper FF13.
        // We use rationals here to prevent numerical instabilities.
        std::vector<SizeType> comp_sizes;
        Rational curr_upper_bound = eps * Rational(Rational(node_cnt, part_cnt).ceil_to_int());
        Rational upper_bound = (Rational(1) + eps) * Rational(Rational(node_cnt, part_cnt).ceil_to_int());
        while (curr_upper_bound < upper_bound) {
            comp_sizes.push_back(static_cast<SizeType>(curr_upper_bound.ceil_to_int()));
            curr_upper_bound *= (Rational(1) + eps);
        }
        comp_sizes.push_back(static_cast<SizeType>(upper_bound.floor_to_int()));
        return comp_sizes;
    }

    SignaturesForTree Tree::cut(Tree::RationalType eps, SizeType part_cnt) {
        std::vector<std::vector<SignatureMap>> signatures;
        for (auto& lvl : this->levels) {
            signatures.emplace_back(lvl.size());
        }

        // Calculate the size intervals of the connected components of a signature.
        std::vector<SizeType> comp_size_bounds = calculate_component_size_bounds(eps, this->tree_sizes[0][0], part_cnt);

        // Iterate over all nodes except the root starting with the node one the bottom left.
        for (size_t lvl_idx = this->levels.size() - 1; lvl_idx > 0; --lvl_idx) {
            for (size_t node_idx = 0; node_idx < this->levels[lvl_idx].size(); ++node_idx) {
                Node const& node = this->levels[lvl_idx][node_idx];
                bool const node_has_left_sibling = this->has_left_sibling[lvl_idx][node_idx];
                bool const node_has_child = node.children_idx_range.first < node.children_idx_range.second;
                SignatureMap empty_map;
                // The only signature which always has cut value smaller infinity(even if the node does not exist) is the 0-vector.
                empty_map[0][Signature(comp_size_bounds.size())] = 0;

                SignatureMap const* left_sibling_sigs = &empty_map;
                SignatureMap const* child_sigs = &empty_map;

                // Adjust the reference to the signatures if the node has a left sibling or
                // has a child respectively.
                // TODO: Think about optimizing the case where there is no left sibling or there is no child.
                // At the moment we are adding the 0-vector to all signatures calculated beforehand which is 
                // unnecessary.
                if (node_has_left_sibling) {
                    left_sibling_sigs = &signatures[lvl_idx][node_idx - 1];
                }
                if (node_has_child) {
                    child_sigs = &signatures[lvl_idx + 1][node.children_idx_range.second - 1];
                }

                // Iterate over all calculated signatures of the left sibling and the rightmost child according
                // to the dynamic programming scheme described in the paper FF13.
                SignatureMap& node_sigs = signatures[lvl_idx][node_idx];
                for (auto const& left_sibling_sigs_with_size : *left_sibling_sigs) {
                    for (auto const& child_sigs_with_size : *child_sigs) {
                        for (auto const& left_sibling_sig : left_sibling_sigs_with_size.second) {
                            for (auto const& child_sig : child_sigs_with_size.second) {
                                // First case: The edge from the current node to its parent is not cut.
                                SizeType frontier_size = left_sibling_sigs_with_size.first + child_sigs_with_size.first;
                                EdgeWeightType cut_cost = left_sibling_sig.second + child_sig.second;
                                Signature sig = left_sibling_sig.first + child_sig.first;
                                if (node_sigs[frontier_size].find(sig) == node_sigs[frontier_size].end()) {
                                    node_sigs[frontier_size][sig] = cut_cost;
                                } else {
                                    node_sigs[frontier_size][sig] = std::min(node_sigs[frontier_size][sig], cut_cost);
                                }

                                // Second case: The edge from the current node to its parent is cut.
                                // Check if the current size of the component which includes the current node is smaller than
                                // the maximum allowed size.
                                SizeType const node_comp_size = this->tree_sizes[lvl_idx][node_idx] - child_sigs_with_size.first;
                                if (node_comp_size >= comp_size_bounds.back()) {
                                    continue;
                                } else {
                                    frontier_size += node_comp_size;
                                    cut_cost += node.parent_edge_weight;

                                    // Adjust the signature to account for the component which contains the current node.
                                    size_t i = 0; 
                                    while (node_comp_size >= comp_size_bounds[i]) { ++i; }
                                    sig[i] += 1;
                                    if (node_sigs[frontier_size].find(sig) == node_sigs[frontier_size].end()) {
                                        node_sigs[frontier_size][sig] = cut_cost;
                                    } else {
                                        node_sigs[frontier_size][sig] = std::min(node_sigs[frontier_size][sig], cut_cost);
                                    }
                                }
                            }
                        }
                    }
                }
            }

            // Calculate the signatures at the root according to the paper FF13. 
            // Signatures which contain less then the total amount of nodes are ignored.
            SignatureMap& root_sigs = signatures[0][0];
            SizeType const node_cnt = this->tree_sizes[0][0];
            for (auto& sigs_with_size : signatures[1].back()) {
                SizeType const node_comp_size = node_cnt - sigs_with_size.first;
                if (node_comp_size >= comp_size_bounds.back()) {
                    continue;
                } else {
                    for (auto& sig : sigs_with_size.second) {
                        Signature root_sig(sig.first);
                        size_t i = 0;
                        while(node_comp_size >= comp_size_bounds[i]) { ++i; }
                        root_sig[i] += 1;
                        if (root_sigs[node_cnt].find(root_sig) == root_sigs[node_cnt].end()) {
                            root_sigs[node_cnt][root_sig] = sig.second;
                        } else {
                            root_sigs[node_cnt][root_sig] = std::min(root_sigs[node_cnt][root_sig], sig.second);
                        }
                    }
                }
            }
        }

        return SignaturesForTree(part_cnt, eps, *this, std::move(signatures));
    }

}
