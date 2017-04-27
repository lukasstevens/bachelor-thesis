#include<fstream>
#include<set>
#include<string>

#include<gtest/gtest.h>

#include "Cut.cpp"

struct AlgorithmParams {
    cut::RationalType eps;
    cut::SizeType part_cnt;

    AlgorithmParams(cut::RationalType eps, cut::SizeType part_cnt) : eps(eps), part_cnt(part_cnt) {}
};

using TestParams = std::pair<std::string, std::string>;
class TestCut : public testing::TestWithParam<TestParams> {};


AlgorithmParams get_algorithm_params(std::string tree_name, std::string params_name) {
    cut::SizeType part_cnt;
    long eps_num;
    long eps_denom;
    std::string params_resource_name("resources/" + tree_name + "." + params_name + ".signatures");
    std::ifstream param_stream(params_resource_name);
    EXPECT_TRUE(param_stream.good()) << "Failed to open file " << params_resource_name << ".";
    param_stream >> part_cnt >> eps_num >> eps_denom;
    param_stream.close();
    cut::RationalType eps(eps_num, eps_denom);
    return AlgorithmParams(eps, part_cnt);
}

cut::Tree get_tree(std::string tree_name) {
    std::string tree_resource_filename = "resources/" + tree_name + ".tree";
    std::ifstream tree_stream(tree_resource_filename);
    EXPECT_TRUE(tree_stream.good()) << "Failed to open file " << tree_resource_filename << ".";
    cut::Tree tree;
    tree_stream >> tree;
    tree_stream.close();
    return tree;
}

TEST_P(TestCut, CutsAsExpected) {
    std::string tree_name = this->GetParam().first;
    std::string params_name = this->GetParam().second;
    std::string signatures_resource_filename = "resources/" + tree_name + "." + params_name + ".signatures";

    AlgorithmParams params = get_algorithm_params(tree_name, params_name);

    cut::Tree tree = get_tree(tree_name);

    std::ifstream signature_stream(signatures_resource_filename);
    ASSERT_TRUE(signature_stream.good()) << "Failed to open file " << signatures_resource_filename << ".";
    cut::SignaturesForTreeBuilder signature_builder(tree);
    signature_stream >> signature_builder;
    signature_stream.close();
    auto should_signatures = signature_builder.finish();

    auto signatures = tree.cut(params.eps, params.part_cnt);
    ASSERT_EQ(should_signatures.signatures.size(), signatures.signatures.size());
    for (size_t lvl_idx = 0; lvl_idx < should_signatures.signatures.size(); ++lvl_idx) {
        ASSERT_EQ(should_signatures.signatures[lvl_idx].size(), signatures.signatures[lvl_idx].size());
        for (size_t node_idx = 0; node_idx < should_signatures.signatures[lvl_idx].size(); ++node_idx) {
            auto const& should_node_sigs = should_signatures.signatures[lvl_idx][node_idx];
            auto const& node_sigs = signatures.signatures[lvl_idx][node_idx];
            EXPECT_EQ(should_node_sigs.size(), node_sigs.size());

            for (auto const& should_node_sigs_with_size : should_node_sigs) {
                EXPECT_NE(node_sigs.find(should_node_sigs_with_size.first), node_sigs.end());
                auto const& node_sigs_with_size = node_sigs.at(should_node_sigs_with_size.first);
                EXPECT_EQ(should_node_sigs_with_size.second.size(), node_sigs_with_size.size());
                for (auto const& should_node_sig : should_node_sigs_with_size.second) {
                    EXPECT_NE(
                            node_sigs_with_size.find(should_node_sig.first),
                            node_sigs_with_size.end() 
                            );
                    EXPECT_EQ(should_node_sig.second, node_sigs_with_size.at(should_node_sig.first));
                }
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

void test_comp_size_bounds(cut::RationalType eps, cut::SizeType node_cnt, cut::SizeType part_cnt, 
        size_t should_length, std::vector<cut::SizeType>& should_upper, std::vector<cut::SizeType>& should_lower) {

    auto upper = cut::calculate_upper_component_size_bounds(eps, node_cnt, part_cnt);

    auto lower = cut::calculate_lower_component_size_bounds(eps, node_cnt, part_cnt);
    ASSERT_EQ(should_length, upper.size());
    ASSERT_EQ(should_upper, upper);
    ASSERT_EQ(should_length, lower.size());
    ASSERT_EQ(should_lower, lower);
}

TEST(ComponentSizeBounds, One) {

    std::vector<cut::SizeType> should_upper_comp_size_bounds({
            9, 13, 20, 26
            });

    std::vector<cut::SizeType> should_lower_comp_size_bounds({
            1, 9 , 13, 20 
            });

    test_comp_size_bounds(cut::RationalType(1, 2), 100, 6, 4, 
            should_upper_comp_size_bounds, should_lower_comp_size_bounds);

}

TEST(ComponentSizeBounds, Two) {

    std::vector<cut::SizeType> should_upper_comp_size_bounds({
            5, 7, 9, 12, 16, 21
            });

    std::vector<cut::SizeType> should_lower_comp_size_bounds({
            1, 5, 7, 9, 12, 16 
            });

    test_comp_size_bounds(cut::RationalType(1, 3), 100, 7, 6, 
            should_upper_comp_size_bounds, should_lower_comp_size_bounds);
}

TEST(ComponentSizeBounds, Three) {

    std::vector<cut::SizeType> should_upper_comp_size_bounds({
            6, 8, 11, 16, 20
            });

    std::vector<cut::SizeType> should_lower_comp_size_bounds({
            1, 6, 8, 11, 16
            });

    test_comp_size_bounds(cut::RationalType(2, 5), 80, 6, 5, 
            should_upper_comp_size_bounds, should_lower_comp_size_bounds);
}

TEST(ComponentSizeBounds, Four) {

    std::vector<cut::SizeType> should_upper_comp_size_bounds({
            6, 8, 12, 17, 19
            });

    std::vector<cut::SizeType> should_lower_comp_size_bounds({
            1, 6, 8, 12, 17
            });

    test_comp_size_bounds(cut::RationalType(3, 7), 121, 10, 5, 
            should_upper_comp_size_bounds, should_lower_comp_size_bounds);
}

TEST(ComponentSizeBounds, Five) {

    std::vector<cut::SizeType> should_upper_comp_size_bounds({
            3, 4, 5, 6, 8, 11, 12
            });

    std::vector<cut::SizeType> should_lower_comp_size_bounds({
            1, 3, 4, 5, 6, 8, 11
            });

    test_comp_size_bounds(cut::RationalType(3, 10), 81, 9, 7, 
            should_upper_comp_size_bounds, should_lower_comp_size_bounds);
}
