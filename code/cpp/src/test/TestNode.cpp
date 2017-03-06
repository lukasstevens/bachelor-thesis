#include<unordered_map>

#include<gtest/gtest.h>

#include "Partition.hpp"

using namespace part;

TEST(BuildTreeTest, TwoNodes) {
    std::unordered_map<Node::IdType, std::unordered_map<Node::IdType, Node::EdgeWeightType>> tree; 
    tree[1][2] = 3;
    Node root = Node::build_tree(tree, 1);
    ASSERT_EQ(root.id, 1);
    ASSERT_EQ(root.parent, nullptr);
    ASSERT_EQ(root.children.size(), 1);
    ASSERT_EQ(root.subtree_size, 2);
    ASSERT_EQ(root.children[0].id, 2);
    ASSERT_EQ(root.children[0].parent, &root);
    ASSERT_EQ(root.children[0].parent_edge_weight, 3);
    ASSERT_EQ(root.children[0].children.size(), 0);
    ASSERT_EQ(root.children[0].subtree_size, 1);
}

TEST(BuildTreeTest, ThreeNodes) {
    std::unordered_map<Node::IdType, std::unordered_map<Node::IdType, Node::EdgeWeightType>> tree; 
    tree[1][2] = 3;
    tree[1][3] = 4;
    Node root = Node::build_tree(tree, 1);
    ASSERT_EQ(root.id, 1);
    ASSERT_EQ(root.children.size(), 2);
    ASSERT_EQ(root.subtree_size, 3);
    Node::IdType two_id = 0;
    Node::IdType three_id = 1;
    if (root.children[0].id != 2) {
        two_id = 1;
        three_id = 0;
    }
    ASSERT_EQ(root.children[two_id].parent_edge_weight, 3);
    ASSERT_EQ(root.children[two_id].subtree_size, 1);
    ASSERT_EQ(root.children[three_id].parent_edge_weight, 4);
    ASSERT_EQ(root.children[three_id].subtree_size, 1);
}

TEST(BuildTreeTest, ThreeNodesChain) {
    std::unordered_map<Node::IdType, std::unordered_map<Node::IdType, Node::EdgeWeightType>> tree; 
    tree[1][2] = 3;
    tree[2][3] = 4;
    Node root = Node::build_tree(tree, 1);
    ASSERT_EQ(root.id, 1);
    ASSERT_EQ(root.children.size(), 1);
    ASSERT_EQ(root.subtree_size, 3);
    ASSERT_EQ(root.children[0].id, 2);
    ASSERT_EQ(root.children[0].subtree_size, 2);
    ASSERT_EQ(root.children[0].parent_edge_weight, 3);
    ASSERT_EQ(root.children[0].children.size(), 1);
    ASSERT_EQ(root.children[0].children[0].id, 3);
    ASSERT_EQ(root.children[0].children[0].subtree_size, 1);
    ASSERT_EQ(root.children[0].children[0].parent_edge_weight, 4);
}
