/** @file Pack.hpp */
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
