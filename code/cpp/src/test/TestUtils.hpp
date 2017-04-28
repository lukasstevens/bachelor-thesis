#pragma once

#include<fstream>
#include<sstream>

#include<gtest/gtest.h>

#include "Cut.hpp"
#include "Partition.hpp"

namespace testutils {

    struct AlgorithmParams {
        cut::RationalType eps;
        cut::SizeType part_cnt;

        AlgorithmParams(cut::RationalType eps, cut::SizeType part_cnt) : eps(eps), part_cnt(part_cnt) {}
    };

    AlgorithmParams get_algorithm_params(std::string tree_name, std::string params_name);

    cut::Tree get_tree(std::string tree_name);

    cut::SignaturesForTree get_signatures_for_tree(std::string tree_name, 
            std::string params_name, cut::Tree const& tree);

    std::tuple<part::Partitioning, cut::Signature, cut::Node::EdgeWeightType> get_opt_partitioning(
            std::string tree_name,
            std::string params_name);

}
