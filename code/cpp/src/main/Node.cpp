#include<cstdint>
#include<vector>
#include<unordered_map>
#include<unordered_set>
#include<string>
#include<sstream>
#include<memory>
#include<list>

#include "Node.hpp"

Node::Node(IdType i, Node* p, EdgeWeightType w) : id(i), parent(p), parent_edge_weight(w) {}

Node Node::build_tree(std::unordered_map<IdType, std::unordered_map<IdType, EdgeWeightType>>& tree) {
    Node root(tree.begin()->first, nullptr, 0);

    std::list<Node*> queue;
    queue.push_back(&root);
    std::unordered_set<IdType> visited;

    while (!queue.empty()) {
        Node* curr_node = queue.front();
        queue.pop_front();
        visited.insert(curr_node->id);

        for (auto& neighbor : tree[curr_node->id]) {
            if (visited.find(neighbor.first) == visited.end()) {
                curr_node->children.emplace_back(neighbor.first, curr_node, neighbor.second);
                queue.push_back(&curr_node->children.back());
            }
        }
    }
    return root;
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

