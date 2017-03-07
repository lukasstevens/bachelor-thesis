#pragma once

#include<cstdint>
#include<limits>
#include<map>
#include<memory>
#include<unordered_map>
#include<string>
#include<vector>

namespace part {

    struct Signature {
        public:
        using CountType = uint32_t;

        std::vector<CountType> sig;

        Signature(std::vector<CountType>);

        friend bool operator==(const Signature& lhs, const Signature& rhs);
        friend inline bool operator!=(const Signature& lhs, const Signature& rhs) {
            return !(lhs == rhs);
        }

        Signature& operator+=(const Signature& rhs);
        friend Signature operator+(Signature lhs, const Signature& rhs);
    };

}

namespace std {

    template <>
    struct hash<part::Signature> {
        size_t operator()(const part::Signature& s) const;
    };

}

namespace part {

    struct Node {
        public:
        using IdType = uint32_t;
        using EdgeWeightType = int32_t;

        IdType const id;
        EdgeWeightType const parent_edge_weight;
        size_t const parent_idx;
        std::pair<size_t const, size_t const> const children_idx_range;

        Node(IdType const id, EdgeWeightType const parent_edge_weight, 
                size_t const parent_idx, std::pair<size_t const, size_t const> const children_idx_range);
    };

    struct Tree {
        public:
        std::vector<std::vector<Node>> levels;
        std::vector<std::vector<bool>> has_left_sibling;

        static Tree build_tree(std::unordered_map<Node::IdType, std::unordered_map<Node::IdType, Node::EdgeWeightType>>& tree, Node::IdType root_id);
        static Tree build_tree(std::unordered_map<Node::IdType, std::unordered_map<Node::IdType, Node::EdgeWeightType>>& tree);
    };
}

