#include<unordered_map>

#include<gtest/gtest.h>

#include "Partition.hpp"

using namespace part;

TEST(BuildTreeTest, TwoNodes) {
    std::unordered_map<Node::IdType, std::unordered_map<Node::IdType, Node::EdgeWeightType>> tree; 
    tree[1][2] = 3;
    Node root = Node::build_tree(tree, 1);
    ASSERT_EQ(root.id, 1) << "Root Id not one.";
    ASSERT_EQ(root.parent, nullptr) << "Root parent is not null.";
    ASSERT_EQ(root.children.size(), 1) << "Root has not one child.";
    ASSERT_EQ(root.children[0].id, 2) << "Only child does not have id 2.";
    ASSERT_EQ(root.children[0].parent, &root) << "Only child has wrong parent pointer.";
    ASSERT_EQ(root.children[0].parent_edge_weight, 3) << "Only child has wrong parent edge weight.";
    ASSERT_EQ(root.children[0].children.size(), 0) << "Only child has children.";
}

TEST(BuildTreeTest, ThreeNodes) {
    std::unordered_map<Node::IdType, std::unordered_map<Node::IdType, Node::EdgeWeightType>> tree; 
    tree[1][2] = 3;
    tree[1][3] = 4;
    Node root = Node::build_tree(tree, 1);
    ASSERT_EQ(root.id, 1) << "Root Id not one.";
    ASSERT_EQ(root.children.size(), 2) << "Root has not two children.";
    Node::IdType two_id = 0;
    Node::IdType three_id = 1;
    if (root.children[0].id != 2) {
        two_id = 1;
        three_id = 0;
    }
    ASSERT_EQ(root.children[two_id].parent_edge_weight, 3) << "Node 2 has not parent edge weight 3.";
    ASSERT_EQ(root.children[three_id].parent_edge_weight, 4) << "Node 3 has not parent edge weight 4.";
}

TEST(BuildTreeTest, ThreeNodesChain) {
    std::unordered_map<Node::IdType, std::unordered_map<Node::IdType, Node::EdgeWeightType>> tree; 
    tree[1][2] = 3;
    tree[2][3] = 4;
    Node root = Node::build_tree(tree, 1);
    ASSERT_EQ(root.id, 1) << "Root Id not one.";
    ASSERT_EQ(root.children.size(), 1) << "Root has not one child.";
    ASSERT_EQ(root.children[0].id, 2) << "Node 2 has not id 2.";
    ASSERT_EQ(root.children[0].parent_edge_weight, 3) << "Node 2 has not parent edge weight 3.";
    ASSERT_EQ(root.children[0].children.size(), 1) << "Node 2 has not one child.";
    ASSERT_EQ(root.children[0].children[0].id, 3) << "Node 3 has not id 3.";
    ASSERT_EQ(root.children[0].children[0].parent_edge_weight, 4) << "Node 3 has not parent_edge_weight 4.";
}
