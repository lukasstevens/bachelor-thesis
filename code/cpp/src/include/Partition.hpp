/** @file Partition.hpp */
#pragma once

#include "Cut.hpp"

/**
 * Contains functions for running the partitioning algorithm described in the paper FF13.
 */
namespace part {

    /**
     * This type represents a partitioning of a tree.
     * Each entry represents one part in the partitioning.
     * One entry is a set of node ids which represents all nodes in that part.
     */
    template<typename IdType>
        using Partitioning = std::vector<std::set<IdType>>;  

    /**
     * Calculates the best feasible packing for the signatures given by \p signatures.
     * @param signatures The signatures and tree to use for the calculations.
     * @returns The components, the best signature and the cut cost as a tuple.
     */
    template<typename IdType, typename EdgeWeightType>
        std::tuple<Partitioning<IdType>, cut::Signature<IdType>, EdgeWeightType> 
            calculate_best_packing(cut::SignaturesForTree<IdType, EdgeWeightType> const& signatures);
}

// Include template implementation file.
#include "Partition.ipp"
