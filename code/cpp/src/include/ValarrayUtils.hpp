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
             * @param s the valarray to hash.
             * @returns The hash.
             */
            size_t operator()(const std::valarray<T>& s) const {
                // Interpret the underlying vector of the signature as a bytestream. Convert the bytestream
                // to a std::string and then hash it with the default hash.
                std::string stream;
                for (T const& value : s) {
                    char const* value_as_chars = static_cast<char const*>(static_cast<void const*>(&value));
                    for (size_t i = 0; i < sizeof(T); ++i) {
                        stream.push_back(value_as_chars[i]);
                    }
                }

                std::hash<std::string> hash_fn;
                return hash_fn(stream);
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
