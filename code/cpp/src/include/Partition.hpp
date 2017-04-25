#pragma once

#include<queue>

#include "Cut.hpp"
#include "GMPUtils.hpp"
#include "Pack.hpp"

/**
 * Contains functions for running the partitioning algorithm described in the paper FF13.
 */
namespace part {

    /**
     * Calculates the best feasible packing for the signatures given by \p signatures.
     * @param signatures The signatures and tree to use for the calculations.
     * @returns The components, the best signature and the cut cost as a tuple.
     */
    std::tuple<std::vector<std::set<cut::Node::IdType>>, cut::Signature, cut::Node::EdgeWeightType> calculate_best_packing(cut::SignaturesForTree const& signatures) {
        auto const& root_sigs = signatures.signatures[0][0].at(signatures.tree.tree_sizes[0][0]);
        using SignatureWithCost = std::pair<cut::Node::EdgeWeightType, cut::Signature>;
        auto compare = [](SignatureWithCost left, SignatureWithCost right){
            return left.first > right.first;
        };

        std::priority_queue<SignatureWithCost, std::vector<SignatureWithCost>, decltype(compare)> prio_q(compare);

        for (auto const& sig : root_sigs) {
            prio_q.emplace(sig.second, sig.first);
        }

        while (!prio_q.empty()) {
            cut::Signature curr_sig;
            cut::Node::EdgeWeightType curr_cut_cost;
            std::tie(curr_cut_cost, curr_sig) = prio_q.top();
            prio_q.pop();

            std::map<cut::SizeType, cut::SizeType> curr_sig_as_map;
            // Starting at 1 to skip components with size smaller than eps * ceil(n/k).
            for (size_t comp_idx = 1; comp_idx < curr_sig.size(); ++comp_idx) {
                if (curr_sig[comp_idx] > 0) {
                    curr_sig_as_map.emplace(signatures.lower_comp_size_bounds[comp_idx], curr_sig[comp_idx]);
                }
            }

            // We substract one from the upper bound since the bounds are exclusive,
            // but the bin capacities are inclusive.
            pack::Packing<cut::SizeType> curr_packing(
                    signatures.lower_comp_size_bounds.back(),
                    signatures.upper_comp_size_bounds.back() - 1);
            curr_packing.pack_perfect(curr_sig_as_map);

            if (curr_packing.bin_cnt() > static_cast<size_t>(signatures.part_cnt)) {
                continue;
            } else {
                auto cut_edges_for_curr_sig = signatures.cut_edges_for_signature(curr_sig);
                auto comps_for_curr_sig = signatures.components_for_cut_edges(cut_edges_for_curr_sig);

                std::map<cut::SizeType, std::vector<cut::SizeType>> expansion_map;
                std::map<cut::SizeType, cut::SizeType> small_components;
                for (auto const& comp : comps_for_curr_sig) {
                    size_t bound_idx = 0;
                    while (comp.size() >= static_cast<size_t>(signatures.upper_comp_size_bounds[bound_idx])) {
                        ++bound_idx;
                    }

                    if (bound_idx == 0) {
                        small_components[static_cast<cut::SizeType>(comp.size())] += 1;
                    } else {
                        expansion_map[signatures.lower_comp_size_bounds[bound_idx]]
                            .push_back(static_cast<cut::SizeType>(comp.size()));
                    }
                }
                curr_packing.expand_packing(expansion_map);
                curr_packing.pack_first_fit(small_components);

                if (curr_packing.bin_cnt() != static_cast<size_t>(signatures.part_cnt)) {
                    continue;
                } else {
                    std::vector<std::vector<cut::SizeType>> bins = curr_packing.get_bins();
                    std::vector<std::set<cut::Node::IdType>> partitioning(bins.size());
                    std::vector<bool> used_comp(comps_for_curr_sig.size());
                    for (size_t bin_idx = 0; bin_idx < bins.size(); ++bin_idx) {
                        for (auto const comp_size : bins[bin_idx]) {
                            for (size_t comp_idx = 0; comp_idx < comps_for_curr_sig.size(); ++comp_idx) {
                                auto const& curr_comp = comps_for_curr_sig[comp_idx];
                                if (static_cast<size_t>(comp_size) == curr_comp.size() &&
                                        !used_comp[comp_idx]) {
                                    used_comp[comp_idx] = true;
                                    partitioning[bin_idx].insert(curr_comp.begin(), curr_comp.end());
                                    break;
                                }
                            }
                        }
                    }
                    return std::make_tuple(partitioning, curr_sig, curr_cut_cost);
                }
            }
        }

        return std::make_tuple(std::vector<std::set<cut::Node::IdType>>(), cut::Signature(), std::numeric_limits<cut::SizeType>::max());
    }
}
