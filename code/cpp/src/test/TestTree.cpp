#include<unordered_map>

#include<gtest/gtest.h>

#include "Partition.hpp"

using namespace part;

TEST(BuildTree, TwoNodes) {
    std::unordered_map<Node::IdType, std::unordered_map<Node::IdType, Node::EdgeWeightType>> tree_map;
    tree_map[1][2] = 3;
    Tree tree = Tree::build_tree(tree_map, 1);
    Node root = tree.levels[0][0];
    Node child = tree.levels[1][0];
    ASSERT_EQ(tree.levels.size(), 2);
    ASSERT_EQ(tree.levels[0].size(), 1);
    ASSERT_EQ(tree.levels[1].size(), 1);
    ASSERT_EQ(root.id, 1);
    ASSERT_EQ(root.parent_idx, 0);
    ASSERT_EQ(root.parent_edge_weight, 0);
    auto should_range_root = std::pair<size_t const, size_t const>(0, 1);
    ASSERT_EQ(root.children_idx_range, should_range_root);
    ASSERT_FALSE(tree.has_left_sibling[0][0]);
    ASSERT_EQ(child.id, 2);
    ASSERT_EQ(child.parent_edge_weight, tree_map[1][2]);
    ASSERT_EQ(child.parent_idx, 0);
    auto should_range_child = std::pair<size_t const, size_t const>(0, 0);
    ASSERT_EQ(child.children_idx_range, should_range_child);
    ASSERT_FALSE(tree.has_left_sibling[0][1]);
}

TEST(BuildTree, ThreeNodeChain) {
    std::unordered_map<Node::IdType, std::unordered_map<Node::IdType, Node::EdgeWeightType>> tree_map;
    tree_map[1][2] = 4;
    tree_map[2][3] = 5;
    Tree tree = Tree::build_tree(tree_map, 1);
    ASSERT_EQ(tree.levels.size(), 3);
    for (auto level : tree.levels) {
        ASSERT_EQ(level.size(), 1);
    }
    Node root = tree.levels[0][0];
    Node fst_child = tree.levels[1][0];
    Node snd_child = tree.levels[2][0];

    Node node_arr[] = {root, fst_child, snd_child};
    for (size_t idx = 0; idx < 3; ++idx) {
        if (idx < 2) {
            ASSERT_EQ(node_arr[idx].children_idx_range.first, 0);
            ASSERT_EQ(node_arr[idx].children_idx_range.second, 1);
        } 
        if (idx == 3) {
            ASSERT_EQ(node_arr[idx].children_idx_range.first, 0);
            ASSERT_EQ(node_arr[idx].children_idx_range.second, 0);
        }
        if (idx > 1) {
            ASSERT_EQ(node_arr[idx].parent_edge_weight, tree_map[node_arr[idx - 1].id][node_arr[idx].id]);
        }
        ASSERT_EQ(node_arr[idx].parent_idx, 0);
        ASSERT_FALSE(tree.has_left_sibling[idx][0]);
    }
}

TEST(BuildTree, ThreeNodes) {
    std::unordered_map<Node::IdType, std::unordered_map<Node::IdType, Node::EdgeWeightType>> tree_map;
    tree_map[1][2] = 4;
    tree_map[1][3] = 5;
    Tree tree = Tree::build_tree(tree_map, 1);
    ASSERT_EQ(tree.levels.size(), 2);
    ASSERT_EQ(tree.levels[0].size(), 1);
    ASSERT_EQ(tree.levels[1].size(), 2);

    Node root = tree.levels[0][0];
    Node fst_child = tree.levels[1][0];
    Node snd_child = tree.levels[1][1];

    ASSERT_EQ(root.children_idx_range.first, 0);
    ASSERT_EQ(root.children_idx_range.second, 2);

    for (auto node : {fst_child, snd_child}) {
        ASSERT_EQ(node.children_idx_range.first, 0);
        ASSERT_EQ(node.children_idx_range.second, 0);
        ASSERT_EQ(node.parent_idx, 0);
        ASSERT_EQ(tree_map[root.id][node.id], node.parent_edge_weight);
    }
    ASSERT_TRUE(tree.has_left_sibling[1][1]);
    ASSERT_FALSE(tree.has_left_sibling[0][0]);
    ASSERT_FALSE(tree.has_left_sibling[0][1]);
}