/** @file ValarrayUtils.hpp */
#pragma once

#include<algorithm>
#include<string>
#include<valarray>

/**
 * Contains utilities to handle valarrays.
 */
namespace valarrutils {

    /**
     * A hasher for valarray.
     * It converts the numbers in the valarray to a string and hashes the string.
     * @tparam T The integer type to use.
     */
    template<typename T>
        struct ValarrayHasher {
            /**
             * Hash a valarray.
             * @param arr the valarray to hash.
             * @returns The hash.
             */
            size_t operator()(const std::valarray<T>& arr) const {
                // For an explanation see http://stackoverflow.com/a/27216842
				std::size_t seed = arr.size();
				for(auto& entry : arr) {
					seed ^= entry + 0x9e3779b9 + (seed << 6) + (seed >> 2);
				}
				return seed;
			}
		};

	/**
	 * Compares two valarrays for equality since the operator == returns a valarray<bool>.
	 * @tparam T The integer type to use.
	 */
	template<typename T>
		struct ValarrayEqual {

			/**
			 * Tests two valarrays for equality.
			 * @param lhs The first valarray.
			 * @param rhs The second valarray.
			 * @returns True if the operands are equal.
			 */
			bool operator()(const std::valarray<T>& lhs, const std::valarray<T>& rhs) const {
				auto comp = lhs == rhs;
				// min returns false if at least one member is false.
				return comp.min();
			}
		};
}
