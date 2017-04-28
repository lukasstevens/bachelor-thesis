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
    using Partitioning = std::vector<std::set<cut::Node::IdType>>;  

    /**
     * Calculates the best feasible packing for the signatures given by \p signatures.
     * @param signatures The signatures and tree to use for the calculations.
     * @returns The components, the best signature and the cut cost as a tuple.
     */
    std::tuple<Partitioning, cut::Signature, cut::Node::EdgeWeightType> calculate_best_packing(cut::SignaturesForTree const& signatures);
}
