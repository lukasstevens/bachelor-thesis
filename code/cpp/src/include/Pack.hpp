#include<map>
#include<valarray>
#include<vector>

// T always has to be a signed number.
namespace pack {

    template<typename T>
        std::vector<std::vector<T>> pack_perfect(std::vector<T> const& component_sizes, std::valarray<T> component_cnts, T const bin_capacity);

    template<typename T>
        std::vector<std::vector<T>> expand_packing(std::vector<std::vector<T>> const& packing, std::map<T, std::pair<T, T>> component_size_mapping);

    template<typename T>
        std::vector<std::vector<T>> pack_first_fit(std::vector<std::vector<T>> const& packing, std::vector<T> components, T const bin_capacity);

}
