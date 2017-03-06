#include<cassert>
#include<cstdint>
#include<list>
#include<sstream>
#include<string>
#include<unordered_map>
#include<unordered_set>
#include<vector>

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

    Node::Node(IdType i, Node* p, EdgeWeightType w) : id(i), parent(p), parent_edge_weight(w), subtree_size(0) {}

    Node Node::build_tree(std::unordered_map<IdType, std::unordered_map<IdType, EdgeWeightType>>& tree, IdType root_id) {
        Node root(root_id, nullptr, 0);
        // This is the BFS queue. It contains the nodes only indirectly by a pointer to the parent node 
        // and the index of the node in the children vector of the parent node.
        // Only pointers to nodes can be in the queue since Nodes can't be copied. Furthermore we cannot save a 
        // pointer directly since the children vectors may be moved to another memory location when resizing.
        std::list<std::pair<size_t, Node*>> queue;
        std::unordered_set<IdType> visited;

        // All nodes which are in the queue have to be in the children vector of the parent. Therefore we
        // have to handle the root seperately since it has no parent.
        Node::IdType child_idx = 0;
        for (auto& child : tree[root.id]) {
            root.children.emplace_back(child.first, &root, child.second);
            queue.push_back(std::make_pair(child_idx, &root));
            child_idx += 1;
        }
        visited.insert(root.id);

        // Do a BFS to discover all descendants of the root and attach children to the root and its 
        // descendants accordingly.
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

        // Visited set has to be cleared when reusing it.
        visited.clear();
        std::list<Node*> stack;
        stack.push_back(&root);

        // Do a post-order traversal to determine the size of the subtree rooted at each node.
        while (!stack.empty()) {
            Node* curr_node = stack.back();

            if (visited.find(curr_node->id) != visited.end()) {
                // Only pop the current node from the stack if it is the second time we
                // encounter it.
                stack.pop_back();
                // Subtree size is initially 1 to account for the current node.
                IdType subtree_size = 1;
                for (auto& child : curr_node->children) {
                    subtree_size += child.subtree_size;
                }
                curr_node->subtree_size = subtree_size;
            } else {
                visited.insert(curr_node->id);
                for (auto& child : curr_node->children) {
                    stack.push_back(&child);
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
