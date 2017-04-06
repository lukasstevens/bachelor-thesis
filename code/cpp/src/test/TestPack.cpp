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
}

