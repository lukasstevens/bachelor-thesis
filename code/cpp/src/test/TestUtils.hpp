#pragma once

#include "Cut.hpp"
#include "Partition.hpp"

namespace testutils {

    template<typename SizeType>
        struct AlgorithmParams {
            cut::Rational eps;
            SizeType part_cnt;

            AlgorithmParams(cut::Rational eps, SizeType part_cnt) : eps(eps), part_cnt(part_cnt) {}
        };

    template<typename SizeType>
        AlgorithmParams<SizeType> get_algorithm_params(std::string tree_name, std::string params_name);

    template<typename IdType, typename EdgeWeightType>
        cut::Tree<IdType, EdgeWeightType> get_tree(std::string tree_name);

    template<typename IdType, typename NodeWeight, typename EdgeWeightType>
        cut::SignaturesForTree<IdType, NodeWeight, EdgeWeightType> get_signatures_for_tree(std::string tree_name, 
                std::string params_name, cut::Tree<IdType, NodeWeight, EdgeWeightType> const& tree);

    template<typename IdType, typename NodeWeight, typename EdgeWeightType>
        std::tuple<part::Partitioning<IdType>, cut::Signature<NodeWeight>, EdgeWeightType> get_opt_partitioning(
                std::string tree_name,
                std::string params_name);

}

// Include template implementation.
#include "TestUtils.ipp"
