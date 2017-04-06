#include<unordered_map>
#include<unordered_set>

#include "ValarrayUtils.hpp"

#include "Pack.hpp"


namespace pack {

    template<typename T>
        std::vector<std::valarray<T>> calculate_bin_signatures(
                std::vector<T> const& component_sizes,
                std::valarray<T> const& component_cnts,
                T const bin_capacity) {
            using Signature = std::valarray<T>;
            using SignatureMap = std::unordered_map<Signature, T, valarrutils::ValarrayHasher<T>, valarrutils::ValarrayEqual<T>>;

            SignatureMap old_bin_signatures;
            old_bin_signatures[Signature(component_cnts.size())] = 0;
            for (size_t idx = 0; idx < component_sizes.size(); ++idx) {
                SignatureMap bin_signatures;
                for (auto const& old_bin_signature : old_bin_signatures) {
                    for (T comp_cnt = 0; comp_cnt < component_cnts[idx]; ++comp_cnt) {
                        Signature bin_signature = old_bin_signature.first;
                        bin_signature[idx] += comp_cnt;
                        T const bin_signature_component_size = old_bin_signature.second + component_sizes[idx];
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
        std::pair<bool, std::valarray<T>> calculate_partial_packing(
                std::unordered_map<std::valarray<T>, std::valarray<T>> const& prev_partial_packing, 
                std::unordered_map<std::valarray<T>, std::valarray<T>>& curr_partial_packing,
                std::vector<std::valarray<T>> const& bin_signatures) {

            for (auto const& prev_packing_signature : prev_partial_packing) {
                for (auto const& bin_signature : bin_signatures) {
                    std::valarray<T> packing_signature = prev_packing_signature.first - bin_signature; 
                    curr_partial_packing[packing_signature] = prev_packing_signature.first;
                    if (packing_signature.max() <= 0) {
                        return std::make_pair(true, packing_signature);
                    }
                }
            }
            return std::make_pair(false, std::valarray<T>());
        }

    template<typename T>
        void Packing<T>::pack_perfect(std::map<T, T> const& components) {

            std::vector<T> component_sizes;
            std::valarray<T> component_cnts;
            for (auto const& component : components) {
                component_sizes.push_back(component.first);
                component_cnts.push_back(component.second);
            }
            
            auto bin_signatures = calculate_bin_signatures(component_sizes, component_cnts, this->bin_capacity);

            std::vector<std::unordered_map<Signature, Signature>> partial_packings(1);
            partial_packings[0][component_cnts] = component_cnts;
            bool found_packing = false;
            Signature complete_packing;
            for(size_t idx = 1; !found_packing; ++idx) {
                partial_packings.push_back(std::unordered_set<Signature>());
                std::tie(found_packing, complete_packing) = 
                    calculate_partial_packing(partial_packings[idx - 1], partial_packings[idx], bin_signatures);
            }

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
            std::map<T, std::pair<T, T>> comp_size_mapping_cp(component_size_mapping);

            for (auto const& bin : this->bins) {
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
                    this->bins.push_back(std::move(std::vector<T>()));
                    this->bins.back().push_back(comp);
                    remaining_bin_capacities.push_back(this->approx_bin_capacity - comp);
                }
            }
        }

}
