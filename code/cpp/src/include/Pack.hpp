#include<map>
#include<valarray>
#include<vector>

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
        };

}
