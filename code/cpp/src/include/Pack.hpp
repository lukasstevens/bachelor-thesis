/** @file Pack.hpp */
#pragma once

#include<map>
#include<valarray>
#include<vector>
#include<unordered_map>

#include "ValarrayUtils.hpp"

/**
 * This namespace contains all classes, functions and type definitions needed for the
 * bin packing approximation algorithm.
 */
namespace pack {

    /**
     * This class represents a packing of objects of different sizes into bins.
     * @tparam T the type which is used to count the objects. This MUST be a signed integer.
     */
    template<typename T>
        struct Packing {
            public:
                T const opt_bin_capacity; /**< The bin capacity for the optimal packing phase */
                T const approx_bin_capacity; /**< The bin capacity for the expansion and greedy phase */

                /**
                 * A type representing the number of objects of specific sizes in a bin.
                 * The sizes are given in Packing::pack_perfect() by \p components.
                 */
                using Signature = std::valarray<T>; 

                /**
                 * Constructor which initializes the packing with the given capacities.
                 * @param opt_bin_capacity The bin capacity in an optimal packing.
                 * @param approx_bin_capacity The relaxed capacity given by the approximation factor.
                 */
                Packing(T opt_bin_capacity, T approx_bin_capacity) : 
                    opt_bin_capacity(opt_bin_capacity), approx_bin_capacity(approx_bin_capacity) {}

                /**
                 * Packs the \p components perfectly using brute-force.
                 * @param components A mapping of component sizes to the number of components of a given size.
                 */
                void pack_perfect(std::map<T, T> const& components);

                /**
                 * Expands the packed components as given by \p component_size_mapping.
                 * @param component_size_mapping A mapping of a component size to several possible expanded sizes.
                 */
                void expand_packing(std::map<T, std::vector<T>> const& component_size_mapping);

                /**
                 * Packs the components given by \p componenents greedily in a first-fit manner.
                 * @param components A mapping of component sizes to the number of components of the given size.
                 */
                void pack_first_fit(std::map<T, T> const& components);

                /** 
                 * Gets the number of bins in the current packing.
                 * @returns The number of bins.
                 */
                size_t bin_cnt() {
                    return this->bins.size();
                }
                
                /**
                 * Gets the current packing.
                 * A bin contains saves the components by their sizes in a vector. These bins are organized in a vector.
                 * @returns The current packing.
                 */
                std::vector<std::vector<T>> get_bins() {
                    return this->bins;
                }

            private:
                std::vector<std::vector<T>> bins; /**< Represents the current packing */

                /**
                 * Calculates signatures which represent all possibilities to pack a single bin.
                 * @param component_sizes Represents the sizes of the components.
                 * @param component_cnts Represents the number of components of a size given by \p component_sizes.
                 * @param bin_capacity The maximum capacity of a bin.
                 * @returns All possible signatures.
                 */
                static std::vector<Signature> calculate_bin_signatures(
                        std::vector<T> const& component_sizes, 
                        Signature const& component_cnts,
                        T const bin_capacity);

                /**
                 * Maps Signatures to Signatures.
                 * This is used to remember the previous signature for a given signature so that
                 * it is possible to calculate which component was packed into which bin.
                 */
                using SignatureToSignatureMap = std::unordered_map<Signature, Signature, 
                      valarrutils::ValarrayHasher<T>, valarrutils::ValarrayEqual<T>>;

                /**
                 * Try all possibilities to pack the current bin and adjust \p current_partial_packing accordingly.
                 * @param prev_partial_packing All possible packings for the previous bins.
                 * @param curr_partial_packing The map into which all packings after packing the current bin are written.
                 * @param bin_signatures All possibilities to pack a single bin.
                 * @returns True if there are no components remaining which have to be packed.
                 */
                static bool calculate_partial_packing(
                        SignatureToSignatureMap const& prev_partial_packing, 
                        SignatureToSignatureMap& curr_partial_packing,
                        std::vector<std::valarray<T>> const& bin_signatures); 

        };
}

namespace pack {

    template<typename T>
        std::vector<std::valarray<T>> Packing<T>::calculate_bin_signatures(
                std::vector<T> const& component_sizes,
                std::valarray<T> const& component_cnts,
                T bin_capacity) {
            using SignatureToTMap = std::unordered_map<Signature, T, valarrutils::ValarrayHasher<T>, valarrutils::ValarrayEqual<T>>;

            SignatureToTMap old_bin_signatures;
            old_bin_signatures[Signature(component_cnts.size())] = 0;
            for (size_t idx = 0; idx < component_sizes.size(); ++idx) {
                SignatureToTMap bin_signatures;
                for (auto const& old_bin_signature : old_bin_signatures) {
                    for (T comp_cnt = 0; comp_cnt <= component_cnts[idx]; ++comp_cnt) {
                        Signature bin_signature = old_bin_signature.first;
                        bin_signature[idx] += comp_cnt;
                        T const bin_signature_component_size = old_bin_signature.second + comp_cnt * component_sizes[idx];
                        if (bin_signature_component_size <= bin_capacity) {
                            bin_signatures[bin_signature] = bin_signature_component_size;
                        } 
                    }
                }
                old_bin_signatures = std::move(bin_signatures);
            }

            std::vector<Signature> bin_signatures;
            for (auto const& bin_signature : old_bin_signatures) {
                bool is_fully_packed = true;
                for (size_t idx = 0; idx < component_cnts.size(); ++idx) {
                    if (bin_signature.first[idx] < component_cnts[idx] && component_sizes[idx] <= bin_capacity - bin_signature.second) {
                        is_fully_packed = false;
                        break;
                    } 
                }
                if (is_fully_packed) {
                    bin_signatures.push_back(bin_signature.first);
                }
            }

            return bin_signatures;
        }

    template<typename T>
        bool Packing<T>::calculate_partial_packing(
                SignatureToSignatureMap const& prev_partial_packing, 
                SignatureToSignatureMap& curr_partial_packing,
                std::vector<std::valarray<T>> const& bin_signatures) {

            for (auto const& prev_packing_signature : prev_partial_packing) {
                for (auto const& bin_signature : bin_signatures) {

                    Signature packing_signature = prev_packing_signature.first - bin_signature; 
                    bool all_comp_cnts_leq_zero = true;
                    for (auto& comp_cnt : packing_signature) {
                        if (comp_cnt <= 0) {
                            comp_cnt = 0;
                        } else {
                            all_comp_cnts_leq_zero = false;
                        }
                    }
                    curr_partial_packing[packing_signature] = prev_packing_signature.first;

                    if (all_comp_cnts_leq_zero) {
                        return true;
                    }
                }
            }
            return false;
        }

    template<typename T>
        void Packing<T>::pack_perfect(std::map<T, T> const& components) {

            std::vector<T> component_sizes(components.size());
            Signature component_cnts(components.size());
            {
                size_t idx = 0;
                for (auto const& component : components) {
                    component_sizes[idx] = component.first;
                    component_cnts[idx] = component.second;
                    idx += 1;
                }
            }

            auto const bin_signatures = calculate_bin_signatures(component_sizes, component_cnts, this->opt_bin_capacity);

            std::vector<std::unordered_map<Signature, Signature, valarrutils::ValarrayHasher<T>, valarrutils::ValarrayEqual<T>>> partial_packings(1);
            partial_packings[0][component_cnts] = component_cnts;
            bool is_packing_complete = false;
            for(size_t idx = 1; !is_packing_complete; ++idx) {
                partial_packings.emplace_back();
                is_packing_complete = calculate_partial_packing(partial_packings[idx - 1], partial_packings[idx], bin_signatures);
            }

            Signature curr_packing(component_sizes.size());
            for (size_t bin_idx = partial_packings.size() - 1; bin_idx > 0; --bin_idx) {
                Signature curr_bin_signature = partial_packings[bin_idx][curr_packing] - curr_packing;

                std::vector<T> curr_bin;
                for (size_t component_idx = 0; component_idx < curr_bin_signature.size(); ++component_idx) {
                    auto back = curr_bin.end();
                    curr_bin.insert(back, curr_bin_signature[component_idx], component_sizes[component_idx]);
                }
                this->bins.push_back(curr_bin);

                curr_packing = partial_packings[bin_idx][curr_packing];
            }
        }

    template<typename T>
        void Packing<T>::expand_packing(std::map<T, std::vector<T>> const& component_size_mapping) {
            std::map<T, std::vector<T>> comp_size_mapping_cp(component_size_mapping);

            for (auto& bin : this->bins) {
                for (auto& comp : bin) {
                    T old_comp = comp;
                    comp = comp_size_mapping_cp[old_comp].back(); 
                    comp_size_mapping_cp[old_comp].pop_back();
                }
            }
        }

    template<typename T>
        void Packing<T>::pack_first_fit(std::map<T, T> const& components) {
            std::vector<T> remaining_bin_capacities;
            for (auto const& bin : this->bins) {
                T filling = 0;
                for (auto const& comp : bin) {
                    filling += comp;
                }
                remaining_bin_capacities.push_back(this->approx_bin_capacity - filling);
            }

            for (auto const& comp : components) {
                for (T comp_idx = 0; comp_idx < comp.second; ++comp_idx) {
                    bool comp_is_packed = false;
                    size_t idx = 0;
                    for (auto& bin : this->bins) {
                        if (comp.first <= remaining_bin_capacities[idx]) {
                            bin.push_back(comp.first);
                            remaining_bin_capacities[idx] -= comp.first;
                            comp_is_packed = true;
                            break;
                        }
                        idx += 1;
                    }
                    if (!comp_is_packed) {
                        this->bins.emplace_back();
                        this->bins.back().push_back(comp.first);
                        remaining_bin_capacities.push_back(this->approx_bin_capacity - comp.first);
                    }
                }
            }
        }

}
