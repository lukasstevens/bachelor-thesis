#include "Pack.hpp"

namespace pack {

    template<typename T>
        std::vector<std::valarray<T>> Packing<T>::calculate_bin_signatures(
                std::vector<T> const& component_sizes,
                std::valarray<T> const& component_cnts,
                T const bin_capacity) {
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
                    if (packing_signature.max() <= 0) {
                        packing_signature = 0;
                        curr_partial_packing[packing_signature] = prev_packing_signature.first;
                        return true;
                    }
                    curr_partial_packing[packing_signature] = prev_packing_signature.first;
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

            auto bin_signatures = calculate_bin_signatures(component_sizes, component_cnts, this->opt_bin_capacity);

            std::vector<std::unordered_map<Signature, Signature, valarrutils::ValarrayHasher<T>, valarrutils::ValarrayEqual<T>>> partial_packings(1);
            partial_packings[0][component_cnts] = component_cnts;
            bool is_packing_complete = false;
            for(size_t idx = 1; !is_packing_complete; ++idx) {
                partial_packings.emplace_back();
                is_packing_complete = calculate_partial_packing(partial_packings[idx - 1], partial_packings[idx], bin_signatures);
            }

            Signature complete_packing(component_sizes.size());
            for (size_t bin_idx = partial_packings.size() - 1; bin_idx > 0; --bin_idx) {
                Signature curr_bin_signature = partial_packings[bin_idx][complete_packing] - complete_packing;
                std::vector<T> curr_bin;
                for (size_t component_idx = 0; component_idx < curr_bin_signature.size(); ++component_idx) {
                    auto back = curr_bin.end();
                    curr_bin.insert(back, curr_bin_signature[component_idx], component_sizes[component_idx]);
                }
                this->bins.push_back(curr_bin);
                complete_packing += curr_bin_signature;
            }
        }

    template<typename T>
        void Packing<T>::expand_packing(std::map<T, std::vector<T>> const& component_size_mapping) {
            std::map<T, std::vector<T>> comp_size_mapping_cp(component_size_mapping);

            for (auto& bin : this->bins) {
                for (auto& comp : bin) {
                    auto old_comp = comp;
                    comp = comp_size_mapping_cp[old_comp].back(); 
                    comp_size_mapping_cp[old_comp].pop_back();
                }
            }
        }

    template<typename T>
        void Packing<T>::pack_first_fit(std::vector<T> const& components) {
            std::vector<T> remaining_bin_capacities;
            for (auto const& bin : this->bins) {
                T filling = 0;
                for (auto const& comp : bin) {
                    filling += comp;
                }
                remaining_bin_capacities.push_back(this->approx_bin_capacity - filling);
            }

            for (auto const& comp : components) {
                bool comp_is_packed = false;
                size_t idx = 0;
                for (auto& bin : this->bins) {
                    if (comp <= remaining_bin_capacities[idx]) {
                        bin.push_back(comp);
                        remaining_bin_capacities[idx] -= comp;
                        comp_is_packed = true;
                        break;
                    }
                    idx += 1;
                }
                if (!comp_is_packed) {
                    this->bins.emplace_back();
                    this->bins.back().push_back(comp);
                    remaining_bin_capacities.push_back(this->approx_bin_capacity - comp);
                }
            }
        }

}
