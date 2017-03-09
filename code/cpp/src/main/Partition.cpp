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

        return tree;
    }

    Tree Tree::build_tree(std::unordered_map<IdType, std::unordered_map<IdType, EdgeWeightType>>& tree_map) {
        return build_tree(tree_map, tree_map.begin()->first);
    }

    std::vector<std::vector<Tree::SignatureMap>> Tree::partition(long double eps, size_t part_cnt) {
        using CountType = Signature::CountType;

        size_t node_cnt = 0;
        std::vector<std::vector<SignatureMap>> signatures;
        for (size_t lvl_idx = 0; lvl_idx < levels.size(); ++lvl_idx) {
            node_cnt += levels[lvl_idx].size();
            signatures.push_back(std::vector<SignatureMap>(levels[lvl_idx].size()));
        }
        size_t const max_comp_size = static_cast<size_t>( std::ceil((1.0L + eps) * std::ceil(static_cast<long double>(node_cnt) / part_cnt) - 1.0L) );

        for (size_t lvl_idx = levels.size() - 1; lvl_idx < levels.size(); --lvl_idx) {
            std::vector<Node>& curr_level = levels[lvl_idx];
            for (size_t node_idx = 0; node_idx < curr_level.size(); ++lvl_idx) {
                Node& curr_node = curr_level[node_idx];
                SignatureMap& curr_node_signatures = signatures[lvl_idx][node_idx];
                bool curr_has_child = curr_node.children_idx_range.first < curr_node.children_idx_range.second; 
                bool curr_has_left_sibling = has_left_sibling[lvl_idx][node_idx];

                if (!curr_has_child) {
                    if (!curr_has_left_sibling) {
                        curr_node_signatures[0][Signature(eps)] = 0;
                        curr_node_signatures[1][Signature(eps, node_cnt, part_cnt, 1)] = curr_node.parent_edge_weight;
                    } else {
                        SignatureMap& left_sibling_signatures = signatures[lvl_idx][node_idx - 1];
                        curr_node_signatures = left_sibling_signatures; 
                        for (auto& sigs_size_m : left_sibling_signatures) {
                            CountType m = sigs_size_m.first;
                            for (CountType n_v = 0; n_v <= max_comp_size; ++n_v) {
                                for (auto& sig_with_val : sigs_size_m.second) {
                                    Signature sig = sig_with_val.first + Signature(eps, node_cnt, part_cnt, n_v);
                                    if (curr_node_signatures[m + n_v].find(sig) != curr_node_signatures[m + n_v].end()) {
                                        curr_node_signatures[m + n_v][sig] = 
                                            std::min(curr_node_signatures[m + n_v][sig], sig_with_val.second + curr_node.parent_edge_weight);
                                    } else {
                                        curr_node_signatures[m + n_v][sig] = sig_with_val.second + curr_node.parent_edge_weight;
                                    }
                                }
                            }
                        }
                    }
                } else {
                    if (!curr_has_left_sibling) {

                    } else {

                    }
                }
            }
        }

        return signatures;
    }

    Signature::Signature(std::vector<Signature::CountType> s) : sig(s) {}

    Signature::Signature(long double eps) : 
        sig(std::vector<Signature::CountType>(static_cast<size_t>(std::ceil(std::log2l(1.0L/eps) / std::log2l(1.0L+eps))) + 1 + 1)) {}

    Signature::Signature(long double eps, size_t node_cnt, size_t part_cnt, Signature::CountType component_size) : Signature(eps) {
        long double node_cnt_l = static_cast<long double>(node_cnt);
        long double part_cnt_l = static_cast<long double>(part_cnt);
        if (component_size < eps * std::ceil(node_cnt_l/part_cnt_l)) {
            sig[0] = 1;
        } else {
            size_t idx = static_cast<size_t>(
                    std::ceil(log2l( component_size / (eps * std::ceil(node_cnt_l/part_cnt_l)) ) / log2l(1.0L + eps) - 1));
            sig[idx] = 1;
        }
    }

    bool operator==(const Signature& lhs, const Signature& rhs) {
        return lhs.sig == rhs.sig;
    }

    Signature& Signature::operator+=(const Signature& rhs) {
        if (this->sig.size() != rhs.sig.size()) {
            throw std::logic_error("Signatures don't have same size.");
        }
        for (size_t i = 0; i < this->sig.size(); ++i) {
            this->sig[i] += rhs.sig[i]; 
        }
        return *this;
    }

    Signature operator+(Signature lhs, const Signature& rhs) {
        return lhs += rhs;
    }
}

namespace std {

    using part::Signature;

    size_t hash<Signature>::operator()(const Signature& s) const {
        using std::hash;
        using std::string;

        // Interpret the underlying vector of the signature as a bytestream. Convert the bytestream
        // to a std::string and then hash it with the default hash.
        string stream;
        for (Signature::CountType const& value : s.sig) {
            char const* value_as_chars = static_cast<char const*>(static_cast<void const*>(&value));
            for (size_t i = 0; i < sizeof(Signature::CountType); ++i) {
                stream.push_back(value_as_chars[i]);
            }
        }

        hash<string> hash_fn;
        return hash_fn(stream);
    }

}
