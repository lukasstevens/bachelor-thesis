#include<cstdint>
#include<vector>
#include<unordered_map>
#include<unordered_set>
#include<string>
#include<sstream>
#include<list>
#include<cassert>

#include "Partition.hpp"

namespace part {

    Signature::Signature(std::vector<Signature::CountType> s) : sig(s) {}

    bool operator==(const Signature& lhs, const Signature& rhs) {
        assert(lhs.sig.size() == rhs.sig.size());
        for (size_t i = 0; i < lhs.sig.size(); ++i) {
            if (lhs.sig[i] != rhs.sig[i]) {
                return false;
            }
        }
        return true;
    }

    Signature& Signature::operator+=(const Signature& rhs) {
        assert(this->sig.size() == rhs.sig.size());
        for (size_t i = 0; i < this->sig.size(); ++i) {
            this->sig[i] += rhs.sig[i]; 
        }
        return *this;
    }

    Signature operator+(Signature lhs, const Signature& rhs) {
        Signature lhs_copy(lhs.sig);
        lhs_copy += rhs;
        return lhs_copy; 
    }
}

namespace std {
    using part::Signature;

    size_t hash<Signature>::operator()(const Signature& s) const {
        using std::hash;
        using std::string;

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

    Node::Node(IdType i, Node* p, EdgeWeightType w) : id(i), parent(p), parent_edge_weight(w) {}

    Node Node::build_tree(std::unordered_map<IdType, std::unordered_map<IdType, EdgeWeightType>>& tree, IdType root_id) {
        Node root(root_id, nullptr, 0);
        std::list<std::pair<size_t, Node*>> queue;
        std::unordered_set<IdType> visited;
        size_t child_idx = 0;
        for (auto& child : tree[root.id]) {
            root.children.emplace_back(child.first, &root, child.second);
            queue.push_back(std::make_pair(child_idx, &root));
            child_idx += 1;
        }
        visited.insert(root.id);

        while (!queue.empty()) {
            Node* parent_node;
            size_t curr_idx;
            std::tie(curr_idx, parent_node) = queue.front();
            Node* curr_node = &parent_node->children[curr_idx];
            queue.pop_front();
            visited.insert(curr_node->id);

            child_idx = 0;
            for (auto& neighbor : tree[curr_node->id]) {
                if (visited.find(neighbor.first) == visited.end()) {
                    curr_node->children.emplace_back(neighbor.first, curr_node, neighbor.second);
                    queue.push_back(std::make_pair(child_idx, curr_node));
                    child_idx += 1;
                }
            }

        }
        return root;
    }


    Node Node::build_tree(std::unordered_map<IdType, std::unordered_map<IdType, EdgeWeightType>>& tree) {
        return build_tree(tree, tree.begin()->first);
    }

    std::string Node::to_string() {
        std::stringstream stream;
        std::list<Node*> queue;
        queue.push_back(this);

        while (!queue.empty()) {
            Node* curr_node = queue.front();
            queue.pop_front();

            for (auto& child : curr_node->children) {
                stream << curr_node->id << " -> " << child.id;
                stream << "[label=\"" << child.parent_edge_weight << "\"]\n";
                queue.push_back(&child);
            }
        }
        return stream.str();
    }
}
