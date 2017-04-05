#include<unordered_map>
#include<unordered_set>

#include "ValarrayUtils.hpp"

#include "Pack.hpp"


namespace pack {

    template<typename T>
        std::pair<bool, std::valarray<T>> calculate_partial_packing(
                std::unordered_map<std::valarray<T>, std::valarray<T>> const& prev_partial_packing, 
                std::unordered_map<std::valarray<T>, std::valarray<T>>& partial_packing,
                std::vector<std::valarray<T>> const& bin_signatures) {

            for (auto const& prev_packing_signature : prev_partial_packing) {
                for (auto const& bin_signature : bin_signatures) {
                    std::valarray<T> packing_signature = prev_packing_signature.first - bin_signature; 
                    partial_packing[packing_signature] = prev_packing_signature.first;
                    if (packing_signature.max() <= 0) {
                        return std::make_pair(true, packing_signature);
                    }
                }
            }
            return std::make_pair(false, std::valarray<T>());
        }

    template<typename T>
        std::vector<std::vector<T>> pack_perfect(std::vector<T> const& component_sizes, std::valarray<T> const& component_cnts, T const bin_capacity) {
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
                            bin_signatures[bin_signature] = old_bin_signature.second + component_sizes[idx];
                        } 
                    }
                }
                old_bin_signatures = std::move(bin_signatures);
            }

            std::vector<Signature> bin_signatures;
            for (auto const& bin_signature : old_bin_signatures) {
                for (size_t idx = 0; idx < component_cnts.size(); ++idx) {
                    if (!(bin_signature.first[idx] < component_cnts[idx] && component_sizes[idx] <= bin_capacity - bin_signature.second)) {
                        bin_signatures.push_back(bin_signature.first); 
                    } 
                }
            }

            std::vector<std::unordered_map<Signature, Signature>> partial_packings(1);
            partial_packings[0][component_cnts] = component_cnts;
            bool found_packing = false;
            Signature complete_packing;
            for(size_t idx = 1; !found_packing; ++idx) {
                partial_packings.push_back(std::unordered_set<Signature>());
                std::tie(found_packing, complete_packing) = 
                    calculate_partial_packing(partial_packings[idx - 1], partial_packings[idx], bin_signatures);
            }

            std::vector<std::vector<T>> packing;
            for (size_t idx = partial_packings.size() - 1; idx > 0; --idx) {
                Signature curr_bin_signature = partial_packings[idx][complete_packing] - complete_packing;
                packing.emplace_back(curr_bin_signature.begin(), curr_bin_signature.end());
                complete_packing += curr_bin_signature;
            }

            return packing;
        }

    // template<typename T>
    //     std::vector<std::vector<T>> expand_packing(std::vector<std::vector<T>> const& packing, std::map<T, std::pair<T, T>> component_size_mapping);

    // template<typename T>
    //     std::vector<std::vector<T>> pack_first_fit(std::vector<std::vector<T>> const& packing, std::vector<T> components, T const bin_capacity);

}
