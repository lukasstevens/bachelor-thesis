#pragma once

#include<cstdint>
#include<map>
#include<unordered_map>
#include<string>
#include<vector>

namespace part {

    struct Signature {
        using CountType = uint32_t;

        public:
        std::vector<CountType> sig;

        Signature(std::vector<CountType>);
        Signature(Signature&&) = default;
        Signature& operator=(Signature&&) = default;
        // Delete copy constructor and copy-assignment so that no accidental copies happen
        Signature(Signature const&) = delete;
        Signature& operator=(Signature const&) = delete;

        friend inline bool operator==(const Signature& lhs, const Signature& rhs);
        friend inline bool operator!=(const Signature& lhs, const Signature& rhs);

        Signature& operator+=(const Signature& rhs);
        friend Signature operator+(Signature lhs, const Signature& rhs);

        friend size_t hash(const Signature& signature);
    };

    struct Node {
        using IdType = uint32_t;
        using EdgeWeightType = uint32_t;

        public:
        IdType const id;
        Node* const parent;
        std::vector<Node> children;
        EdgeWeightType const parent_edge_weight;
        std::map<Signature::CountType, std::unordered_map<Signature, Signature::CountType>> signatures;

        Node(IdType, Node*, EdgeWeightType);
        Node(Node&&) = default;
        Node& operator=(Node&&) = default;
        // Delete copy constructor and copy-assignment so that no accidental copies happen
        Node(Node const&) = delete;
        Node& operator=(Node const&) = delete;

        static Node build_tree(std::unordered_map<IdType, std::unordered_map<IdType, EdgeWeightType>>& tree);
        static Node build_tree(std::unordered_map<IdType, std::unordered_map<IdType, EdgeWeightType>>& tree, IdType root_id);

        std::string to_string();
    };

}

