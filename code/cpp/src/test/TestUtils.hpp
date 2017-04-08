#include<iostream>
#include<string>
#include<unordered_map>

#include "Cut.hpp"

using cut::Node;
using cut::RationalType;
using cut::SizeType;
using cut::Tree;

namespace testutils {
    struct AlgorithmParameters {
        SizeType node_cnt;
        Node::IdType root_id;
        SizeType part_cnt;
        RationalType eps;
        std::unordered_map<Node::IdType, std::unordered_map<Node::IdType, Node::EdgeWeightType>> tree;

        friend std::istream& operator>>(std::istream& is, AlgorithmParameters& params) {
            is >> params.node_cnt >> params.root_id >> params.part_cnt;
            int64_t eps_num;
            int64_t eps_denom;
            is >> eps_num >> eps_denom;
            params.eps = RationalType(eps_num, eps_denom);

            for (SizeType edge_idx = 0; edge_idx < params.node_cnt - 1; ++edge_idx) {
                Node::IdType from; 
                Node::IdType to; 
                Node::EdgeWeightType weight;
                is >> from >> to >> weight;
                params.tree[from][to] = weight;
            }

            return is;
        }
    };
}
