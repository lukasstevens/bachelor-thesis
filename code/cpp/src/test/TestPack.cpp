#include<gtest/gtest.h>

#include "Pack.cpp"

using namespace pack;

TEST(PackFirstFit, One) {
    Packing<int> packing(3, 3);

    std::vector<int> components({ 1, 2, 2, 2, 1 });
    packing.pack_first_fit(components);
    auto bins = packing.get_bins();
    ASSERT_EQ(bins, std::vector<std::vector<int>>({{1, 2}, {2, 1}, {2}}));
}

TEST(PackPerfect, One) {
    Packing<int> packing(5, 7);
    std::map<int, int> components;
    components[1] = 3;
    components[4] = 2;
    components[3] = 1;
    packing.pack_perfect(components);
    ASSERT_EQ(3, packing.get_bins().size());

    std::map<int, std::vector<int>> comp_size_mapping;
    comp_size_mapping[1] = { 1, 1, 1};
    comp_size_mapping[4] = { 4, 5 };
    comp_size_mapping[3] = { 4 };
    packing.expand_packing(comp_size_mapping);

    for (auto const& bin : packing.get_bins()) {
        int bin_fill = 0;
        for(auto const& comp : bin) {
            bin_fill += comp;
        }
        ASSERT_GE(packing.approx_bin_capacity, bin_fill);
    }
}

