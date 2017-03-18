#pragma once

#include<cstdint>
#include<limits>
#include<map>
#include<unordered_map>
#include<string>
#include<vector>

#include "Rational.hpp"

namespace part {

    using SizeType = uint32_t;

    struct Node {
        public:
        using IdType = SizeType;
        using EdgeWeightType = int32_t;

        IdType const id;
        EdgeWeightType const parent_edge_weight;
        size_t const parent_idx;
        std::pair<size_t const, size_t const> const children_idx_range;

        Node(IdType const id, EdgeWeightType const parent_edge_weight, 
                size_t const parent_idx, std::pair<size_t const, size_t const> const children_idx_range);
    };


    struct Signature {
        public:
        using CountType = SizeType;

        std::vector<CountType> sig;

        Signature(std::vector<CountType> vec);
        Signature(size_t length);

        friend bool operator==(const Signature& lhs, const Signature& rhs);
        friend inline bool operator!=(const Signature& lhs, const Signature& rhs) {
            return !(lhs == rhs);
        }

        Signature& operator+=(const Signature& rhs);
        friend Signature operator+(Signature lhs, const Signature& rhs);
    };

    struct Tree {
        public:
        std::vector<std::vector<Node>> levels;
        std::vector<std::vector<bool>> has_left_sibling;
        std::vector<std::vector<SizeType>> tree_sizes;

        static Tree build_tree(std::unordered_map<Node::IdType, std::unordered_map<Node::IdType, Node::EdgeWeightType>>& tree, Node::IdType root_id);
        static Tree build_tree(std::unordered_map<Node::IdType, std::unordered_map<Node::IdType, Node::EdgeWeightType>>& tree);

        using SignatureMap = std::map<Signature::CountType, std::unordered_map<Signature, Node::EdgeWeightType>>;
        using RationalType = rat::Rational<int64_t>;

        std::vector<std::vector<SignatureMap>> partition(RationalType eps, SizeType part_cnt);

        static std::vector<SizeType> calculate_component_size_bounds(RationalType eps, SizeType node_cnt, SizeType part_cnt);
    };
}

namespace std {

    template <>
    struct hash<part::Signature> {
        size_t operator()(const part::Signature& s) const;
    };

}
