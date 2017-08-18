#include<cstdint>
#include<fstream>
#include<set>
#include<string>

#include<gtest/gtest.h>

#include "Cut.hpp"
#include "TestUtils.hpp"

using TestParams = std::pair<std::string, std::string>;
class TestCut : public testing::TestWithParam<TestParams> {};


TEST_P(TestCut, CutsAsExpected) {
    using Id = int32_t;
    using EdgeWeight = int32_t;
    using Tree = cut::Tree<Id, EdgeWeight>;

    std::string tree_name = this->GetParam().first;
    std::string params_name = this->GetParam().second;

    auto params = testutils::get_algorithm_params<Tree::SizeType>(tree_name, params_name);
    Tree tree = testutils::get_tree<Id, EdgeWeight>(tree_name);
    auto should_signatures = testutils::get_signatures_for_tree<Id, EdgeWeight>(tree_name, params_name, tree);

    auto signatures = tree.cut(params.eps, params.part_cnt);
    ASSERT_EQ(should_signatures.signatures.size(), signatures.signatures.size());
    for (size_t lvl_idx = 0; lvl_idx < should_signatures.signatures.size(); ++lvl_idx) {
        ASSERT_EQ(should_signatures.signatures[lvl_idx].size(), signatures.signatures[lvl_idx].size());
        for (size_t node_idx = 0; node_idx < should_signatures.signatures[lvl_idx].size(); ++node_idx) {
            auto const& should_node_sigs = should_signatures.signatures[lvl_idx][node_idx];
            auto const& node_sigs = signatures.signatures[lvl_idx][node_idx];
            EXPECT_EQ(should_node_sigs.size(), node_sigs.size());

            Tree::SizeType node_sigs_size = 0;
            for (auto const& should_node_sigs_with_size : should_node_sigs) {
                auto const& node_sigs_with_size = node_sigs.at(static_cast<size_t>(node_sigs_size));
                EXPECT_EQ(should_node_sigs_with_size.size(), node_sigs_with_size.size());
                for (auto const& should_node_sig : should_node_sigs_with_size) {
                    EXPECT_NE(
                            node_sigs_with_size.find(should_node_sig.first),
                            node_sigs_with_size.end() 
                            );
                    EXPECT_EQ(should_node_sig.second, node_sigs_with_size.at(should_node_sig.first));
                }
                ++node_sigs_size;
            }
        }
    }
}

INSTANTIATE_TEST_CASE_P(
        CutTests,
        TestCut,
        testing::Values(
            TestParams("1", "1"),
            TestParams("2", "1"),
            TestParams("3", "1"),
            TestParams("3", "2"),
            TestParams("4", "1"),
            TestParams("5", "1"),
            TestParams("6", "1")
            ));

template<typename SizeType>
void test_comp_size_bounds(cut::RationalType eps, SizeType node_cnt, SizeType part_cnt, 
        size_t should_length, std::vector<SizeType>& should_upper, std::vector<SizeType>& should_lower) {

    auto upper = cut::calculate_upper_component_size_bounds(eps, node_cnt, part_cnt);

    auto lower = cut::calculate_lower_component_size_bounds(eps, node_cnt, part_cnt);
    ASSERT_EQ(should_length, upper.size());
    ASSERT_EQ(should_upper, upper);
    ASSERT_EQ(should_length, lower.size());
    ASSERT_EQ(should_lower, lower);
}

TEST(ComponentSizeBounds, One) {
    using SizeType = int32_t;

    std::vector<SizeType> should_upper_comp_size_bounds({
            9, 13, 20, 26
            });

    std::vector<SizeType> should_lower_comp_size_bounds({
            1, 9 , 13, 20 
            });

    test_comp_size_bounds(cut::RationalType(1, 2), 100, 6, 4, 
            should_upper_comp_size_bounds, should_lower_comp_size_bounds);

}

TEST(ComponentSizeBounds, Two) {
    using SizeType = int32_t;

    std::vector<SizeType> should_upper_comp_size_bounds({
            5, 7, 9, 12, 16, 21
            });

    std::vector<SizeType> should_lower_comp_size_bounds({
            1, 5, 7, 9, 12, 16 
            });

    test_comp_size_bounds(cut::RationalType(1, 3), 100, 7, 6, 
            should_upper_comp_size_bounds, should_lower_comp_size_bounds);
}

TEST(ComponentSizeBounds, Three) {
    using SizeType = int32_t;

    std::vector<SizeType> should_upper_comp_size_bounds({
            6, 8, 11, 16, 20
            });

    std::vector<SizeType> should_lower_comp_size_bounds({
            1, 6, 8, 11, 16
            });

    test_comp_size_bounds(cut::RationalType(2, 5), 80, 6, 5, 
            should_upper_comp_size_bounds, should_lower_comp_size_bounds);
}

TEST(ComponentSizeBounds, Four) {
    using SizeType = int32_t;

    std::vector<SizeType> should_upper_comp_size_bounds({
            6, 8, 12, 17, 19
            });

    std::vector<SizeType> should_lower_comp_size_bounds({
            1, 6, 8, 12, 17
            });

    test_comp_size_bounds(cut::RationalType(3, 7), 121, 10, 5, 
            should_upper_comp_size_bounds, should_lower_comp_size_bounds);
}

TEST(ComponentSizeBounds, Five) {
    using SizeType = int32_t;

    std::vector<SizeType> should_upper_comp_size_bounds({
            3, 4, 5, 6, 8, 11, 12
            });

    std::vector<SizeType> should_lower_comp_size_bounds({
            1, 3, 4, 5, 6, 8, 11
            });

    test_comp_size_bounds(cut::RationalType(3, 10), 81, 9, 7, 
            should_upper_comp_size_bounds, should_lower_comp_size_bounds);
}
