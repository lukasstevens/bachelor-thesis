#pragma once

#include<cstdint>
#include<limits>
#include<map>
#include<unordered_map>
#include<string>
#include<valarray>
#include<vector>

#include "Rational.hpp"
#include "ValarrayUtils.hpp"

namespace cut {

    using SizeType = int32_t;

    struct Node {
        public:
            using IdType = SizeType;
            using EdgeWeightType = int32_t;

            IdType const id;
            EdgeWeightType const parent_edge_weight;
            size_t const parent_idx;
            std::pair<size_t const, size_t const> const children_idx_range;

            Node(IdType const id, EdgeWeightType const parent_edge_weight, 
                    size_t const parent_idx, std::pair<size_t const, size_t const> const& children_idx_range);
    };

    using Signature = std::valarray<SizeType>;

    struct Tree {
        public:
            std::vector<std::vector<Node>> levels;
            std::vector<std::vector<bool>> has_left_sibling;
            std::vector<std::vector<SizeType>> tree_sizes;

            static Tree build_tree(std::unordered_map<Node::IdType, std::unordered_map<Node::IdType, Node::EdgeWeightType>> const& tree, Node::IdType root_id);
            static Tree build_tree(std::unordered_map<Node::IdType, std::unordered_map<Node::IdType, Node::EdgeWeightType>> const& tree);

            using SignatureMap = std::map<SizeType, std::unordered_map<Signature, Node::EdgeWeightType, 
                  valarrutils::ValarrayHasher<SizeType>, valarrutils::ValarrayEqual<SizeType>>>;
            using RationalType = rat::Rational<int64_t>;

            std::vector<std::vector<SignatureMap>> cut(RationalType const& eps, SizeType const part_cnt);

            static std::vector<SizeType> calculate_component_size_bounds(RationalType const& eps, SizeType const node_cnt, SizeType const part_cnt);
    };
}

