#pragma once

#include<cstdint>
#include<memory>
#include<unordered_map>
#include<string>
#include<vector>

struct Node {
    using IdType = uint32_t;
    using EdgeWeightType = uint32_t;
    public:
        IdType id;
        Node* parent;
        std::vector<Node> children;
        EdgeWeightType parent_edge_weight;

        Node(IdType, Node*, EdgeWeightType);

        static Node build_tree(std::unordered_map<IdType, std::unordered_map<IdType, EdgeWeightType>>& tree);
        static Node build_tree(std::unordered_map<IdType, std::unordered_map<IdType, EdgeWeightType>>& tree, IdType root_id);

        std::string to_string();

        Node(Node&&) = default;
        Node& operator=(Node&&) = default;
        // Delete copy constructor and copy-assignment so that no accidental copies happen
        Node(Node const&) = delete;
        Node& operator=(Node const&) = delete;
};

