#pragma once

#include<algorithm>
#include<string>
#include<valarray>

namespace valarrutils {

    template<typename T>
        struct ValarrayHasher {
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

    template<typename T>
        struct ValarrayEqual {
            bool operator()(const std::valarray<T>& lhs, const std::valarray<T>& rhs) const {
                auto comp = lhs == rhs;
                // min returns false if at least one member is false.
                return comp.min();
            }
        };
}
