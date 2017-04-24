/** @file Cut.hpp */
#pragma once

#include<cstdint>
#include<iostream>
#include<map>
#include<set>
#include<unordered_map>
#include<unordered_set>
#include<valarray>
#include<vector>

#include<gmpxx.h>

#include "ValarrayUtils.hpp"

/** 
 * This namespace contains all classes, functions and type definitions which are needed for the cutting phase.
 */
namespace cut {

    using SizeType = int32_t; /**< The type used for counting nodes */

    /**
     * This class represents a node in the tree.
     */
    struct Node {
        public:
            using IdType = SizeType; /**< The type for identifying a node */
            using EdgeWeightType = int32_t; /**< The type of the edge weights in the tree */

            IdType const id; /**< The id of a node */
            /** 
             * The weight of the edge which connects a node to its parent.
             * If there is no parent node the value of this member is arbitrary
             */
            EdgeWeightType const parent_edge_weight; 
            size_t const parent_idx; /**< The index of the parent of a node in the level above */
            /**
             * The children of a node described by their indices in the level below this node.
             * This range is right exclusive. If the range is empty then the node has no children.
             */
            std::pair<size_t const, size_t const> const children_idx_range; 

            /**
             * Constructor.
             * @param id Id of the node.
             * @param parent_edge_weight The weight of the edge of this node to its parent.
             * @param parent_idx The index of the parent in the level above this nodes.
             * @param children_idx_range The range of indices in which children of this are located
             * in the level below this node.
             */
            Node(IdType id, EdgeWeightType parent_edge_weight, 
                    size_t parent_idx, std::pair<size_t, size_t> children_idx_range);
    };

    using Signature = std::valarray<SizeType>; /**< The type of a signature. **/
    using RationalType = mpq_class; /**< The type of a rational. **/

    struct SignaturesForTree;

    /**
     * This class represents a tree. The nodes of a tree are arranged in levels.
     * The nodes contain the necessary information to determine the parent and the
     * children of a node in the level above and below respectively.
     */
    struct Tree {
        public:
            std::vector<std::vector<Node>> levels; /**< The levels of the tree. */
            std::vector<std::vector<bool>> has_left_sibling; /**< Lookup table saving if a node has a left sibling */
            std::vector<std::vector<SizeType>> tree_sizes; /**< Lookup table for the size of the subtree rooted at a node */

            /**
             * Constructor.
             */
            Tree() = default;

            /**
             * This functions builds a Tree from \p tree. 
             * This is a map of maps which constitutes
             * a projection from two nodes(an edge) onto an edge weight. The tree represented by \p tree is explored in
             * a BFS order starting at \p root_id.
             * @param tree The tree to use.
             * @param root_id The id of the root in the tree.
             * @returns The tree built from \p tree.
             */
            static Tree build_tree(std::map<Node::IdType, std::map<Node::IdType, Node::EdgeWeightType>> const& tree, Node::IdType root_id);

            /**
             * Similiar to the overloaded function. Uses a random root. 
             * For this to work the tree must be undirected.
             * @param tree The tree to use.
             * @returns the tree built from \p tree.
             *
             * @see build_tree()
             */
            static Tree build_tree(std::map<Node::IdType, std::map<Node::IdType, Node::EdgeWeightType>> const& tree);

            /**
             * A type which saves the signatures at a node. 
             * It maps the number of nodes in the lower frontier to the possible 
             * signatures which this number of nodes and the signatures are mapped again to their cut cost.
             */
            using SignatureMap = std::map<SizeType, std::unordered_map<Signature, Node::EdgeWeightType, 
                  valarrutils::ValarrayHasher<SizeType>, valarrutils::ValarrayEqual<SizeType>>>;

            /**
             * Cuts the tree with the given parameters.
             * @param eps The approximation factor to use.
             * @param part_cnt The number of parts in which the tree should be partitioned.
             * @returns The signatures calculated for the given parameters.
             *
             * @see SignaturesForTree
             */
            SignaturesForTree cut(RationalType eps, SizeType part_cnt);

            /**
             * Calculates the node_idx for the node with the id \p node_id.
             * @param node_id The id of the node.
             * @returns A pair describing the index of the node in the levels of the tree.
             * Returns <code>(std::numeric_limits<size_t>::max(), std::numeric_limits<size_t>::max())</code>
             * if no node with id \p node_id exists.
             */
            std::pair<size_t, size_t> get_node_idx(Node::IdType node_id) const;
    };

    /**
     * Populates a Tree with the values from the inputstream.
     * @param is The inputstream.
     * @param tree The tree to build populate.
     * @returns The inputstream.
     */
    std::istream& operator>>(std::istream& is, Tree& tree);

    /**
     * Prints a Tree to the outputstream.
     * @param os The outputstream.
     * @param tree The tree to print.
     * @returns The outputstream.
     */
    std::ostream& operator<<(std::ostream& os, Tree const& tree);
    

    /**
     * Calculates the upper(exclusive) bounds on the component sizes for each index of a signature.
     * @param eps The approximation factor.
     * @param node_cnt The number of nodes in the tree.
     * @param part_cnt The number of parts in which the tree should be partitioned.
     * @returns The component size bounds as a vector.
     *
     * @see calculate_lower_component_size_bounds()
     */
    std::vector<SizeType> calculate_upper_component_size_bounds(RationalType eps, SizeType node_cnt, SizeType part_cnt);

    /**
     * Calculates the lower(inclusive) bounds on the component sizes for each index of a signature.
     * @param eps The approximation factor.
     * @param node_cnt The number of nodes in the tree.
     * @param part_cnt The number of parts in which the tree should be partitioned.
     * @returns The component size bounds as a vector.
     *
     * @see calculate_upper_component_size_bounds()
     */
    std::vector<SizeType> calculate_lower_component_size_bounds(RationalType eps, SizeType node_cnt, SizeType part_cnt);

    /**
     * This class represents the signatures for a tree caclulated by Tree::cut(). 
     * The tree instance MUST outlive the SignatuesForTree instance.
     */
    struct SignaturesForTree {
        public:
            SizeType const part_cnt; /**< The number of parts in the partition. */
            RationalType const eps; /**< The approximation parameter. */
            Tree const& tree; /**< The tree for which the signatures were calculated. */
            std::vector<std::vector<Tree::SignatureMap>> const signatures; /**< The calculated signatures. */
            std::vector<SizeType> const upper_comp_size_bounds; /**< The upper bounds for the sizes in a signature. */
            std::vector<SizeType> const lower_comp_size_bounds; /**< The lower bounds for the sizes in a signature. */

            /**
             * Constructor.
             * @param part_cnt The number of parts in the partition.
             * @param eps The approximation parameter.
             * @param tree The tree for which the signatures were calculated.
             * @param signatures The signatures for \p tree.
             */
            SignaturesForTree(SizeType part_cnt, RationalType eps, Tree const& tree, std::vector<std::vector<Tree::SignatureMap>> signatures) :
                part_cnt(part_cnt), eps(eps), tree(tree), signatures(signatures), 
                upper_comp_size_bounds(calculate_upper_component_size_bounds(eps, tree.tree_sizes[0][0], part_cnt)),
                lower_comp_size_bounds(calculate_lower_component_size_bounds(eps, tree.tree_sizes[0][0], part_cnt))
                    {}


            /** 
             * A type representing the edges cut by the cutting phase.
             */
            using CutEdges = std::set<std::pair<Node::IdType, Node::IdType>>; 

            /**
             * Calculates the edges which were cut to arrive at \p signature at the root.
             * @param signature The signature for which the cut edges should be calculated.
             * @return The edges cut as a set of pairs. One pair specifies the two endpoints of the cut edge.
             */
            CutEdges cut_edges_for_signature(Signature const& signature) const;

            /**
             * Calculates the connected components which result in cutting the edges described by \p cut_edges.
             * @param cut_edges The edges to cut.
             * @returns A vector of sets. Each set identifies one connected component in the Tree by the node ids in that set.
             */
            std::vector<std::set<Node::IdType>> components_for_cut_edges(CutEdges const& cut_edges) const;
    };

    /**
     * Prints the signatures to the outputstream.
     * @param os The outputstream.
     * @param signatures The signatures to print.
     * @returns The outputstream.
     */
    std::ostream& operator<<(std::ostream& os, SignaturesForTree const& signatures);


    /**
     * This class is used to build a SignaturesForTree object.
     * We need this class since the members of the SignaturesForTree class are constant.
     */
    struct SignaturesForTreeBuilder {
        public:
            /** 
             * The tree for which the signatures were calculated. 
             */
            Tree const& tree;

            /**
             * Constructor.
             */
            SignaturesForTreeBuilder(Tree const& tree) : tree(tree) {}

            /**
             * Sets the part count.
             * @param part_cnt The part count.
             * @returns A reference to this.
             */
            SignaturesForTreeBuilder& with_part_cnt(SizeType part_cnt);

            /**
             * Sets the approximation parameter.
             * @param eps The approximation parameter
             * @returns A reference to this.
             */
            SignaturesForTreeBuilder& with_eps(RationalType eps);

            /**
             * Sets the signatures.
             * @param signatures The signatures
             * @returns A reference to this.
             */
            SignaturesForTreeBuilder& with_signatures(std::vector<std::vector<Tree::SignatureMap>> const& signatures);

            /**
             * Finishes building the signatures.
             * @returns The finished SignaturesForTree object.
             */
            SignaturesForTree finish();

        private:
            SizeType part_cnt; /**< The number of parts in the partition. */
            RationalType eps; /**< The approximation parameter. */
            std::vector<std::vector<Tree::SignatureMap>> signatures; /**< The calculated signatures. */

    };

    /**
     * Populates a SignaturesForTreeBuilder with the values from the inputstream.
     * @param is The inputstream.
     * @param builder The builder.
     * @returns The inputstream.
     */
    std::istream& operator>>(std::istream& is, SignaturesForTreeBuilder& builder);
}

