#include<queue>

#include "GMPUtils.hpp"
#include "Pack.hpp"

namespace part {

template<typename IdType, typename EdgeWeightType>
    std::tuple<Partitioning<IdType>, cut::Signature<IdType>, EdgeWeightType> 
        calculate_best_packing(cut::SignaturesForTree<IdType, EdgeWeightType> const& signatures) {

        using Signature = cut::Signature<IdType>;
        using SizeType = IdType;

        auto const& root_sigs = signatures.signatures[0][0].at(signatures.tree.tree_sizes[0][0]);
        using SignatureWithCost = std::pair<EdgeWeightType, Signature>;
        auto compare = [](SignatureWithCost left, SignatureWithCost right){
            return left.first > right.first;
        };

        std::priority_queue<SignatureWithCost, std::vector<SignatureWithCost>, decltype(compare)> prio_q(compare);

        for (auto const& sig : root_sigs) {
            prio_q.emplace(sig.second, sig.first);
        }

        while (!prio_q.empty()) {
            Signature curr_sig;
            EdgeWeightType curr_cut_cost;
            std::tie(curr_cut_cost, curr_sig) = prio_q.top();
            prio_q.pop();

            std::map<SizeType, SizeType> curr_sig_as_map;
            // Starting at 1 to skip components with size smaller than eps * ceil(n/k).
            for (size_t comp_idx = 1; comp_idx < curr_sig.size(); ++comp_idx) {
                if (curr_sig[comp_idx] > 0) {
                    curr_sig_as_map.emplace(signatures.lower_comp_size_bounds[comp_idx], curr_sig[comp_idx]);
                }
            }

            // We substract one from the upper bound since the bounds are exclusive,
            // but the bin capacities are inclusive.
            pack::Packing<SizeType> curr_packing(
                    signatures.lower_comp_size_bounds.back(),
                    signatures.upper_comp_size_bounds.back() - 1);
            curr_packing.pack_perfect(curr_sig_as_map);

            if (curr_packing.bin_cnt() > static_cast<size_t>(signatures.part_cnt)) {
                continue;
            } else {
                auto cut_edges_for_curr_sig = signatures.cut_edges_for_signature(curr_sig);
                auto comps_for_curr_sig = signatures.components_for_cut_edges(cut_edges_for_curr_sig);

                std::map<SizeType, std::vector<SizeType>> expansion_map;
                std::map<SizeType, SizeType> small_components;
                for (auto const& comp : comps_for_curr_sig) {
                    size_t bound_idx = 0;
                    while (comp.size() >= static_cast<size_t>(signatures.upper_comp_size_bounds[bound_idx])) {
                        ++bound_idx;
                    }

                    if (bound_idx == 0) {
                        small_components[static_cast<SizeType>(comp.size())] += 1;
                    } else {
                        expansion_map[signatures.lower_comp_size_bounds[bound_idx]]
                            .push_back(static_cast<SizeType>(comp.size()));
                    }
                }
                curr_packing.expand_packing(expansion_map);
                curr_packing.pack_first_fit(small_components);

                if (curr_packing.bin_cnt() > static_cast<size_t>(signatures.part_cnt)) {
                    continue;
                } else {
                    std::vector<std::vector<SizeType>> bins = curr_packing.get_bins();
                    std::vector<std::set<IdType>> partitioning(bins.size());
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

        return std::make_tuple(std::vector<std::set<IdType>>(), Signature(), std::numeric_limits<SizeType>::max());
    }
}
