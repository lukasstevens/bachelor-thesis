#pragma once

#include<cstdint>
#include<map>
#include<set>
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
    using RationalType = rat::Rational<int64_t>;

    struct SignaturesForTree;

    struct Tree {
        public:
            std::vector<std::vector<Node>> levels;
            std::vector<std::vector<bool>> has_left_sibling;
            std::vector<std::vector<SizeType>> tree_sizes;

            static Tree build_tree(std::unordered_map<Node::IdType, std::unordered_map<Node::IdType, Node::EdgeWeightType>> const& tree, Node::IdType root_id);
            static Tree build_tree(std::unordered_map<Node::IdType, std::unordered_map<Node::IdType, Node::EdgeWeightType>> const& tree);

            using SignatureMap = std::map<SizeType, std::unordered_map<Signature, Node::EdgeWeightType, 
                  valarrutils::ValarrayHasher<SizeType>, valarrutils::ValarrayEqual<SizeType>>>;

            SignaturesForTree cut(RationalType eps, SizeType part_cnt);
    };
    
    
    std::vector<SizeType> calculate_component_size_bounds(RationalType eps, SizeType node_cnt, SizeType part_cnt);

    // Recognize that the Tree object referenced in this object MUST outlive this object.
    struct SignaturesForTree {
        public:
            SizeType const part_cnt;
            RationalType const eps;
            Tree const& tree;
            std::vector<std::vector<Tree::SignatureMap>> const signatures;

            SignaturesForTree(SizeType part_cnt, RationalType eps, Tree const& tree, std::vector<std::vector<Tree::SignatureMap>> signatures) :
                part_cnt(part_cnt), eps(eps), tree(tree), signatures(signatures) {}

            std::set<std::pair<Node::IdType, Node::IdType>> cut_edges_for_signature(Signature const& signature);
    };
}

