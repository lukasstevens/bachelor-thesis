#include<queue>
#include<stdexcept>

#include "GMPUtils.hpp"
#include "Pack.hpp"

namespace part {

template<typename Id, typename NodeWeight, typename EdgeWeight>
    std::tuple<Partitioning<Id>, cut::Signature<Id>, EdgeWeight> 
        calculate_best_packing(cut::SignaturesForTree<Id, NodeWeight, EdgeWeight> const& signatures) {

        using Signature = cut::Signature<Id>;

        using SignatureWithCost = std::pair<EdgeWeight, Signature>;
        auto compare = [](SignatureWithCost left, SignatureWithCost right){
            return left.first > right.first;
        };

        std::priority_queue<SignatureWithCost, std::vector<SignatureWithCost>, decltype(compare)> prio_q(compare);

        auto const& root_sigs = signatures.signatures[0][0].back();
        for (auto const& sig : root_sigs) {
            prio_q.emplace(sig.second, sig.first);
        }

        while (!prio_q.empty()) {
            Signature curr_sig;
            EdgeWeight curr_cut_cost;
            std::tie(curr_cut_cost, curr_sig) = prio_q.top();
            prio_q.pop();

            std::map<NodeWeight, NodeWeight> curr_sig_as_map;
            // Starting at 1 to skip components with size smaller than eps * ceil(n/k).
            for (size_t comp_idx = 1; comp_idx < curr_sig.size(); ++comp_idx) {
                if (curr_sig[comp_idx] > 0) {
                    curr_sig_as_map.emplace(signatures.lower_comp_weight_bounds[comp_idx], curr_sig[comp_idx]);
                }
            }

            // We substract one from the upper bound since the bounds are exclusive,
            // but the bin capacities are inclusive.
            pack::Packing<NodeWeight> curr_packing(
                    signatures.lower_comp_weight_bounds.back(),
                    signatures.upper_comp_weight_bounds.back() - 1
                    );
            curr_packing.pack_perfect(curr_sig_as_map);

            if (curr_packing.bin_cnt() > static_cast<size_t>(signatures.part_cnt)) {
                continue;
            } else {
                auto cut_edges_for_curr_sig = signatures.cut_edges_for_signature(curr_sig);
                auto comps_for_curr_sig = signatures.components_for_cut_edges(cut_edges_for_curr_sig);

                std::map<NodeWeight, std::vector<NodeWeight>> expansion_map;
                std::map<NodeWeight, NodeWeight> small_components;
                std::vector<NodeWeight> comp_weights;
                for (auto const& comp : comps_for_curr_sig) {
                    NodeWeight comp_weight = 0;
                    for (auto const& node : comp) {
                        comp_weight += node.second;
                    }
                    comp_weights.push_back(comp_weight);
                }

                for (size_t comp_idx = 0; comp_idx < comps_for_curr_sig.size(); ++comp_idx) {
                    NodeWeight const comp_weight = comp_weights[comp_idx];
                    size_t bound_idx = 0;
                    while (comp_weight >= signatures.upper_comp_weight_bounds[bound_idx]) {
                        ++bound_idx;
                    }

                    if (bound_idx == 0) {
                        small_components[comp_weight] += 1;
                    } else {
                        expansion_map[signatures.lower_comp_weight_bounds[bound_idx]]
                            .push_back(comp_weight);
                    }
                }
                curr_packing.expand_packing(expansion_map);
                curr_packing.pack_first_fit(small_components);

                if (curr_packing.bin_cnt() > static_cast<size_t>(signatures.part_cnt)) {
                    continue;
                } else {
                    std::vector<std::vector<NodeWeight>> bins = curr_packing.get_bins();
                    std::vector<std::set<Id>> partitioning(bins.size());
                    std::vector<bool> used_comp(comps_for_curr_sig.size());
                    for (size_t bin_idx = 0; bin_idx < bins.size(); ++bin_idx) {
                        for (auto const comp_weight : bins[bin_idx]) {
                            for (size_t comp_idx = 0; comp_idx < comps_for_curr_sig.size(); ++comp_idx) {
                                auto const& curr_comp = comps_for_curr_sig[comp_idx];
                                if (comp_weight == comp_weights[comp_idx] &&
                                        !used_comp[comp_idx]) {
                                    used_comp[comp_idx] = true;
                                    for (auto const& node : curr_comp) {
                                        partitioning[bin_idx].insert(node.first);
                                    }
                                    break;
                                }
                            }
                        }
                    }
                    return std::make_tuple(partitioning, curr_sig, curr_cut_cost);
                }
            }
        }

        throw PartitionException();
    }
}
