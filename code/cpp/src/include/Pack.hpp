#include<map>
#include<valarray>
#include<vector>
#include<unordered_map>

#include "ValarrayUtils.hpp"

// T always has to be a signed number.
namespace pack {

    template<typename T>
        struct Packing {
            public:
                T const opt_bin_capacity;
                T const approx_bin_capacity;

                using Signature = std::valarray<T>;

                Packing(T const opt_bin_capacity, T const approx_bin_capacity) : 
                    opt_bin_capacity(opt_bin_capacity), approx_bin_capacity(approx_bin_capacity) {}

                void pack_perfect(std::map<T, T> const& components);
                void expand_packing(std::map<T, std::vector<T>> const& component_size_mapping);
                void pack_first_fit(std::vector<T> const& components);

                std::vector<std::vector<T>> get_bins() {
                    return this->bins;
                }

            private:
                std::vector<std::vector<T>> bins;

                static std::vector<std::valarray<T>> calculate_bin_signatures(
                        std::vector<T> const& component_sizes, 
                        Signature const& component_cnts,
                        T const bin_capacity);

                using SignatureToSignatureMap = std::unordered_map<Signature, Signature, 
                      valarrutils::ValarrayHasher<T>, valarrutils::ValarrayEqual<T>>;

                static bool calculate_partial_packing(
                        SignatureToSignatureMap const& prev_partial_packing, 
                        SignatureToSignatureMap& curr_partial_packing,
                        std::vector<std::valarray<T>> const& bin_signatures); 

        };
}
