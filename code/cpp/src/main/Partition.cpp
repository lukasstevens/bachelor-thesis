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

    Signature::Signature(std::vector<Signature::CountType> s) : sig(s) {}

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
}
