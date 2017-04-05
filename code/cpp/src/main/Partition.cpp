#include<cmath>
#include<cstdint>
#include<list>
#include<sstream>
#include<stdexcept>
#include<string>
#include<unordered_map>
#include<unordered_set>
#include<vector>

#include "Partition.hpp"

namespace part {

    using EdgeWeightType = Node::EdgeWeightType;
    using IdType = Node::IdType;

    Node::Node(IdType const id, EdgeWeightType const parent_edge_weight, 
            size_t const parent_idx, std::pair<size_t const, size_t const> const children_idx_range) : 
        id(id), parent_edge_weight(parent_edge_weight), parent_idx(parent_idx), children_idx_range(children_idx_range) {}


    Tree Tree::build_tree(std::unordered_map<IdType, std::unordered_map<IdType, EdgeWeightType>>& tree_map, IdType root_id) {
        Tree tree;
        struct NodeStub {
            IdType const id; 
            EdgeWeightType const parent_edge_weight;
            size_t const parent_idx;
            bool const has_left_sibling;
            size_t const level;

            NodeStub(IdType const id, EdgeWeightType const p_e_w, size_t const p_i, bool const h_l_s, size_t const lvl) :
                id(id), parent_edge_weight(p_e_w), parent_idx(p_i), has_left_sibling(h_l_s), level(lvl) {}
        };

        std::list<NodeStub> queue;
        queue.push_back(NodeStub(root_id, 0, 0, false, 0));

        size_t next_child_idx = 0;
        while(!queue.empty()) {
            NodeStub curr_node = queue.front();
            queue.pop_front();

            if (curr_node.level == tree.levels.size()) {
                tree.levels.push_back(std::vector<Node>());
                tree.has_left_sibling.push_back(std::vector<bool>());
                next_child_idx = 0;
            }

            size_t old_next_child_idx = next_child_idx;
            for (auto neighbor : tree_map[curr_node.id]) {
                if (curr_node.level == 0 || neighbor.first != tree.levels[curr_node.level - 1][curr_node.parent_idx].id) {
                    bool has_left_sibling = !(old_next_child_idx == next_child_idx);
                    queue.emplace_back(neighbor.first, neighbor.second, tree.levels[curr_node.level].size(), has_left_sibling, curr_node.level + 1); 
                    ++next_child_idx;
                }
            }

            tree.levels[curr_node.level].emplace_back(curr_node.id, curr_node.parent_edge_weight, 
                    curr_node.parent_idx, std::make_pair(old_next_child_idx, next_child_idx));
            tree.has_left_sibling[curr_node.level].push_back(curr_node.has_left_sibling);

        }

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

    Tree Tree::build_tree(std::unordered_map<IdType, std::unordered_map<IdType, EdgeWeightType>>& tree_map) {
        return build_tree(tree_map, tree_map.begin()->first);
    }

    std::vector<SizeType> Tree::calculate_component_size_bounds(Tree::RationalType eps, SizeType node_cnt, SizeType part_cnt) {
        using Rational = Tree::RationalType;

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

    std::vector<std::vector<Tree::SignatureMap>> Tree::partition(Tree::RationalType eps, SizeType part_cnt) {
        std::vector<std::vector<SignatureMap>> signatures;
        for (auto& lvl : this->levels) {
            signatures.emplace_back(lvl.size());
        }
        
        std::vector<SizeType> comp_size_bounds = calculate_component_size_bounds(eps, this->tree_sizes[0][0], part_cnt);

        for (size_t lvl_idx = this->levels.size() - 1; lvl_idx > 0; --lvl_idx) {
            for (size_t node_idx = 0; node_idx < this->levels[lvl_idx].size(); ++node_idx) {
                Node const& node = this->levels[lvl_idx][node_idx];
                bool const node_has_left_sibling = this->has_left_sibling[lvl_idx][node_idx];
                bool const node_has_child = node.children_idx_range.first < node.children_idx_range.second;
                SignatureMap empty_map;
                empty_map[0][Signature(comp_size_bounds.size())] = 0;
                
                SignatureMap const* left_sibling_sigs = &empty_map;
                SignatureMap const* child_sigs = &empty_map;

                if (node_has_left_sibling) {
                    left_sibling_sigs = &signatures[lvl_idx][node_idx - 1];
                }
                if (node_has_child) {
                    child_sigs = &signatures[lvl_idx + 1][node.children_idx_range.second - 1];
                }

                SignatureMap& node_sigs = signatures[lvl_idx][node_idx];
                for (auto const& left_sibling_sigs_with_size : *left_sibling_sigs) {
                    for (auto const& child_sigs_with_size : *child_sigs) {
                        for (auto const& left_sibling_sig : left_sibling_sigs_with_size.second) {
                            for (auto const& child_sig : child_sigs_with_size.second) {
                                SizeType frontier_size = left_sibling_sigs_with_size.first + child_sigs_with_size.first;
                                EdgeWeightType cut_cost = left_sibling_sig.second + child_sig.second;
                                Signature sig = left_sibling_sig.first + child_sig.first;
                                if (node_sigs[frontier_size].find(sig) == node_sigs[frontier_size].end()) {
                                    node_sigs[frontier_size][sig] = cut_cost;
                                } else {
                                    node_sigs[frontier_size][sig] = std::min(node_sigs[frontier_size][sig], cut_cost);
                                }

                                SizeType const node_comp_size = this->tree_sizes[lvl_idx][node_idx] - child_sigs_with_size.first;
                                if (node_comp_size >= comp_size_bounds.back()) {
                                    continue;
                                } else {
                                    frontier_size += node_comp_size;
                                    cut_cost += node.parent_edge_weight;
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

        return signatures;
    }

}
