#include<cstdint>

#include <gtest/gtest.h>

#include "Cut.hpp"
#include "Pack.hpp"
#include "Partition.hpp"
#include "TestUtils.hpp"

using TestParams = std::pair<std::string, std::string>;
class TestPack : public testing::TestWithParam<TestParams> {};

TEST_P(TestPack, PacksAsExpected) {
    using Id = int32_t;
    using EdgeWeight = int32_t;
    using Tree = cut::Tree<Id, EdgeWeight>;

    std::string tree_name;
    std::string params_name;
    std::tie(tree_name, params_name) = this->GetParam();

    Tree tree = testutils::get_tree<Id, EdgeWeight>(tree_name);
    cut::SignaturesForTree<Id, int, EdgeWeight> signatures 
        = testutils::get_signatures_for_tree<Id, EdgeWeight>(tree_name, params_name, tree);

    part::Partitioning<int> partitioning;
    Tree::Signature best_signature;
    EdgeWeight opt_cut_cost;
    std::tie(partitioning, best_signature, opt_cut_cost) = part::calculate_best_packing(signatures);

    part::Partitioning<int> should_partitioning;
    Tree::Signature should_best_signature;
    EdgeWeight should_opt_cut_cost;
    std::tie(should_partitioning, should_best_signature, should_opt_cut_cost) = 
        testutils::get_opt_partitioning<Id, EdgeWeight>(tree_name, params_name);

    EXPECT_EQ(should_partitioning, partitioning);
    EXPECT_EQ(should_opt_cut_cost, opt_cut_cost);
    valarrutils::ValarrayEqual<int> valarr_eq;
    EXPECT_TRUE(valarr_eq(should_best_signature, best_signature));
}


INSTANTIATE_TEST_CASE_P(
        PackTests,
        TestPack,
        testing::Values(
            TestParams("1", "1"),
            TestParams("2", "1"),
            TestParams("3", "1"),
            TestParams("3", "2"),
            TestParams("4", "1"),
            TestParams("5", "1"),
            TestParams("6", "1")
            ));

TEST(PackFirstFit, One) {
    pack::Packing<int> packing(3, 3);

    std::map<int, int> components;

    components[2] = 1;
    packing.pack_first_fit(components);
    ASSERT_EQ(packing.get_bins(), std::vector<std::vector<int>>({{2}}));

    components[1] = 2;
    components[2] = 3;
    packing.pack_first_fit(components);
    ASSERT_EQ(packing.get_bins(), std::vector<std::vector<int>>({{2, 1}, {1, 2}, {2}, {2}}));
}

TEST(PackPerfect, One) {
    pack::Packing<int> packing(5, 7);
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

