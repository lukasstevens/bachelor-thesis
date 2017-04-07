#pragma once

#include<cstdint>
#include<map>
#include<unordered_map>
#include<valarray>
#include<vector>

#include<gtest/gtest_prod.h>

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

            Node(IdType id, EdgeWeightType parent_edge_weight, 
                    size_t parent_idx, std::pair<size_t, size_t> children_idx_range);
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

            std::vector<std::vector<SignatureMap>> cut(RationalType eps, SizeType part_cnt);

        private:
            FRIEND_TEST(Run, DISABLED_FromStdinVerbose);
            static std::vector<SizeType> calculate_component_size_bounds(RationalType eps, SizeType node_cnt, SizeType part_cnt);
    };
}

