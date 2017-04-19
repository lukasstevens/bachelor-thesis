#pragma once

#include<cstdint>
#include<map>
#include<set>
#include<unordered_map>
#include<unordered_set>
#include<valarray>
#include<vector>

#include<gmpxx.h>

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
    using RationalType = mpq_class;

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
    
    
    std::vector<SizeType> calculate_upper_component_size_bounds(RationalType eps, SizeType node_cnt, SizeType part_cnt);

    std::vector<SizeType> calculate_lower_component_size_bounds(RationalType eps, SizeType node_cnt, SizeType part_cnt);

    // Recognize that the Tree object referenced in this object MUST outlive this object.
    struct SignaturesForTree {
        public:
            SizeType const part_cnt;
            RationalType const eps;
            Tree const& tree;
            std::vector<std::vector<Tree::SignatureMap>> const signatures;
            std::vector<SizeType> const upper_comp_size_bounds;
            std::vector<SizeType> const lower_comp_size_bounds;

            SignaturesForTree(SizeType part_cnt, RationalType eps, Tree const& tree, std::vector<std::vector<Tree::SignatureMap>> signatures) :
                part_cnt(part_cnt), eps(eps), tree(tree), signatures(signatures), 
                upper_comp_size_bounds(calculate_upper_component_size_bounds(eps, tree.tree_sizes[0][0], part_cnt)),
                lower_comp_size_bounds(calculate_lower_component_size_bounds(eps, tree.tree_sizes[0][0], part_cnt))
                    {}


            using CutEdges = std::set<std::pair<Node::IdType, Node::IdType>>;
            CutEdges cut_edges_for_signature(Signature const& signature) const;

            std::vector<std::set<Node::IdType>> components_for_cut_edges(CutEdges const& cut_edges) const;
    };
}

