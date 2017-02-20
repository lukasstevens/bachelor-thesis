#include<unordered_map>

#include<gtest/gtest.h>

#include<Node.hpp>

TEST(BuildTreeTest, TwoNodes) {
    std::unordered_map<Node::IdType, std::unordered_map<Node::IdType, Node::EdgeWeightType>> tree; 
    tree[1][2] = 3;
    Node root = Node::build_tree(tree);
    ASSERT_EQ(root.id, 1) << "Root Id not one.";
    ASSERT_EQ(root.parent, nullptr) << "Root parent is not null.";
    ASSERT_EQ(root.children.size(), 1) << "Root has not one child.";
    ASSERT_EQ(root.children[0].id, 2) << "Only child does not have id 2.";
    ASSERT_EQ(root.children[0].parent, &root) << "Only child has wrong parent pointer.";
    ASSERT_EQ(root.children[0].parent_edge_weight, 3) << "Only child has wrong parent edge weight.";
}
