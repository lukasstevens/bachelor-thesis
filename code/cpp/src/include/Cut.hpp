/** @file Cut.hpp */
#pragma once

#include<cstdint>
#include<iostream>
#include<list>
#include<map>
#include<set>
#include<sstream>
#include<unordered_map>
#include<unordered_set>
#include<valarray>
#include<vector>

#include<gmpxx.h>

#include "GMPUtils.hpp"
#include "ValarrayUtils.hpp"

/** 
 * This namespace contains all classes, functions and type definitions which are needed for the cutting phase.
 * Almost all structs and functions in this namespace use the template parameters Id and EdgeWeight.
 * Id is the type for node ids and EdgeWeight is the type with which edge weights of the Tree are stored.
 * NodeWeight is the type for storing node weights.
 * All types are assumed to be integer, while NodeWeight must be integer. Non-integer EdgeWeight might work,
 * but is not tested.
 * @see Node
 * @see Tree
 */
namespace cut {


    /**
     * This class represents a node in the tree.
     * Id is the type of the node id and EdgeWeight is the type for storing the edge weights.
     * NodeWeight is the type for storing node weights.
     */
    template<typename Id, typename NodeWeight, typename EdgeWeight>
        struct Node {
            public:
                Id const id; /**< The id of a node */
                NodeWeight const weight; /**< The weight of a node */
                /** 
                 * The weight of the edge which connects a node to its parent.
                 * If there is no parent node the value of this member is arbitrary
                 */
                EdgeWeight const parent_edge_weight; 
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
                Node(Id id, EdgeWeight parent_edge_weight, NodeWeight weight, 
                        size_t parent_idx, std::pair<size_t, size_t> children_idx_range);
        };

    template<typename NodeWeight>
        using Signature = std::valarray<NodeWeight>; /**< The type of a signature. **/

    /**
     * A type which saves the signatures at a node. 
     * It maps the number of nodes in the lower frontier to the possible 
     * signatures with this number of nodes and the signatures are mapped to their cut cost.
     * Since signatures are valarrays, we can use ValarrayHasher and ValarrayEqual.
     * @see ValarrayHasher
     * @see ValarrayEqual
     * @see Signature
     */
    template<typename NodeWeight, typename EdgeWeight>
        using SignatureMap = std::vector<std::unordered_map<Signature<NodeWeight>, EdgeWeight, 
              valarrutils::ValarrayHasher<NodeWeight>, valarrutils::ValarrayEqual<NodeWeight>>>;

    /**
     * This saves the signatures which were used to arrive the current signature.
     * @see SignatureMapWithPrev
     */
    template<typename NodeWeight>
        struct PreviousSignatures {
            using Signature = Signature<NodeWeight>;
            std::pair<NodeWeight, Signature> left_sibling_sig;
            std::pair<NodeWeight, Signature> right_child_sig;
            bool was_parent_edge_cut;

            PreviousSignatures() = default;

            PreviousSignatures(
                    std::pair<NodeWeight, Signature> left_sibling_sig,
                    std::pair<NodeWeight, Signature> right_child_sig,
                    bool was_parent_edge_cut) :
                left_sibling_sig(left_sibling_sig), right_child_sig(right_child_sig), 
                was_parent_edge_cut(was_parent_edge_cut) {}
        };

    /**
     * A type which saves the signatures at a node and also the previous signature for each signature. 
     * This is used in cut_edges_for_signature().
     * @see Tree::SignatureMap
     */
    template<typename NodeWeight, typename EdgeWeight>
        using SignatureMapWithPrev = std::map<NodeWeight, 
              std::unordered_map<Signature<NodeWeight>, std::pair<EdgeWeight, PreviousSignatures<NodeWeight>>,
              valarrutils::ValarrayHasher<NodeWeight>, valarrutils::ValarrayEqual<NodeWeight>>>;

    using Rational = mpq_class; /**< The type of a rational. **/

    template<typename Idtype, typename NodeWeight, typename EdgeWeight>
        struct SignaturesForTree;

    /**
     * This class represents a tree. The nodes of a tree are arranged in levels.
     * The nodes contain the necessary information to determine the parent and the
     * children of a node in the level above and below respectively.
     * For an explanation of the types 
     * @see Node
     */
    template<typename Id=int32_t, typename NodeWeight=int32_t, typename EdgeWeight=int32_t>
        struct Tree {
            public:
                using Node = Node<Id, NodeWeight, EdgeWeight>; /**< Type of a Node according to template parameters */
                using Signature = Signature<NodeWeight>; /**< Type of a signature. */
                using SignatureMap = SignatureMap<NodeWeight, EdgeWeight>; /**< The type to save the signatures at a node */
                /** Similar to SignatureMap only with information about previous signatures. */
                using SignatureMapWithPrev = SignatureMapWithPrev<NodeWeight, EdgeWeight>;

                std::vector<std::vector<Node>> levels; /**< The levels of the tree. */
                std::vector<std::vector<bool>> has_left_sibling; /**< Lookup table saving if a node has a left sibling */
                std::vector<std::vector<NodeWeight>> subtree_weight; /**< Lookup table for the size of the subtree rooted at a node */

                /**
                 * Constructor.
                 */
                Tree() = default;

                /**
                 * This functions builds a Tree from \p tree. 
                 * This is a map of maps which constitutes
                 * a projection from two nodes(an edge) onto an edge weight. The tree represented by \p tree is explored in
                 * a BFS order starting at \p root_id.
                 * It is assumed that for each edge (u,v) the edge (v,u) is also in the map with the same weight.
                 *
                 * @param tree The tree to use.
                 * @param node_weight The node weights of the tree.
                 * @param root_id The id of the root in the tree.
                 * @returns The tree built from \p tree.
                 */
                static Tree<Id, NodeWeight, EdgeWeight> build_tree(
                        std::map<Id, std::map<Id, EdgeWeight>> const& tree, 
                        std::map<Id, NodeWeight> const& node_weight, 
                        Id root_id);

                /**
                 * Calculates the sizes of the subtrees in the tree.
                 * It stores the sizes in the member Tree::subtree_weight.
                 */
                void calculate_subtree_weights();


                /**
                 * Cuts the tree with the given parameters.
                 * @param eps The approximation factor to use.
                 * @param part_cnt The number of parts in which the tree should be partitioned.
                 * @param delete_signatures Indicates if the signatures at the nodes except the root
                 *  should be delete if they are not needed any more.
                 * @returns The signatures calculated for the given parameters.
                 *
                 * @see SignaturesForTree
                 */
                SignaturesForTree<Id, NodeWeight, EdgeWeight> cut(Rational eps, NodeWeight part_cnt,
                        bool delete_signatures=false);

                /**
                 * Calculates the signatures of the tree with information about the previous signatures.
                 * Additionally no signatures greater than \p signature are allowed.
                 * Analogous to Tree::cut().
                 */
                std::vector<std::vector<SignatureMapWithPrev>> 
                    cut_with_prev(Rational eps, NodeWeight part_cnt, Signature const& signature) const;

                /**
                 * Calculates the node_idx for the node with the id \p node_id.
                 * @param node_id The id of the node.
                 * @returns A pair describing the index of the node in the levels of the tree.
                 * Returns <code>(std::numeric_limits<size_t>::max(), std::numeric_limits<size_t>::max())</code>
                 * if no node with id \p node_id exists.
                 */
                std::pair<size_t, size_t> get_node_idx(Id node_id) const;

                /**
                 * Outputs the tree in the graphviz format.
                 * The tree is presented exactly as it is layed out in the memory.
                 * @returns The tree in graphviz format as a string.
                 */
                std::string as_graphviz() const;

            private:
                /**
                 * Calculates the signatures at a node.
                 * Uses the signatures of the left sibling and the right child of the node.
                 * If the left sibling or right child does not exist one just has to pass a Tree::SignatureMap
                 * which only contains one signature, namely the 0-vector, and which has cut cost 0.
                 * @param node The current node.
                 * @param node_subtree_size The size of the subtree rooted at the current node.
                 * @param left_siblings_size The combined size of the trees rooted at the siblings of \p node.
                 * @param left_sibling_sigs The signatures at the left sibling.
                 * @param right_child_sigs The signatures at the right child.
                 * @param comp_weight_bounds The upper component size bounds(exclusive) for the signature.
                 */
                static SignatureMap cut_at_node(
                        Node const& node, 
                        NodeWeight node_subtree_weight,
                        NodeWeight left_siblings_weight,
                        SignatureMap const& left_sibling_sigs, 
                        SignatureMap const& right_child_sigs, 
                        std::vector<NodeWeight> const& comp_weight_bounds);


                /**
                 * Calculates the signatures at a node.
                 * This works analogous to Tree::cut_at_node(), only that the previous signatures are calculated
                 * and no signatures greater than \p signature are allowed.
                 * @param node The current node.
                 * @param subtree_size The size of the subtree rooted at the current node.
                 * @param left_sibling_sigs The signatures at the left sibling.
                 * @param right_child_sigs The signatures at the right child.
                 * @param comp_weight_bounds The upper component size bounds(exclusive) for the signature.
                 * @param The maximum allowed signature.
                 */
                static SignatureMapWithPrev cut_at_node_with_prev(
                        Node const& node, 
                        NodeWeight node_subtree_weight,
                        SignatureMapWithPrev const& left_sibling_sigs, 
                        SignatureMapWithPrev const& right_child_sigs,
                        std::vector<NodeWeight> const& comp_weight_bounds,
                        Signature const& signature
                        ); 
        };

    /**
     * Populates a Tree with the values from the inputstream.
     * This uses the following format:
     * The first line contains two integers \c n \c r.
     * \c n specifies the number of nodes in the tree whereas \c r specifies the id of the root.
     * <code>n-1</code> lines follow each containing three integers \c f \c t \c w. 
     * \c f and \c t describe an edge in the tree from \c f to \c t. 
     * The weight of the edge is specified by \c w.
     * @param is The inputstream.
     * @param tree The tree to build populate.
     * @returns The inputstream.
     */
    template<typename Id, typename NodeWeight, typename EdgeWeight>
        std::istream& operator>>(std::istream& is, Tree<Id, NodeWeight, EdgeWeight>& tree);

    /**
     * Prints a Tree to the outputstream.
     * @see std::istream& operator>>(std::istream&, Tree&) The format for outputting a tree.
     * @param os The outputstream.
     * @param tree The tree to print.
     * @returns The outputstream.
     */
    template<typename Id, typename NodeWeight, typename EdgeWeight>
        std::ostream& operator<<(std::ostream& os, Tree<Id, NodeWeight, EdgeWeight> const& tree);

    /**
     * Calculates the upper (exclusive) bounds on the component sizes for each index of a signature.
     * @param eps The approximation factor.
     * @param node_cnt The number of nodes in the tree.
     * @param part_cnt The number of parts in which the tree should be partitioned.
     * @returns The component size bounds as a vector.
     *
     * @see calculate_lower_component_weight_bounds()
     */
    template<typename NodeWeight>
        std::vector<NodeWeight> calculate_upper_component_weight_bounds(
                Rational eps, NodeWeight tree_weight, NodeWeight part_cnt);

    /**
     * Calculates the lower (inclusive) bounds on the component sizes for each index of a signature.
     * @param eps The approximation factor.
     * @param node_cnt The number of nodes in the tree.
     * @param part_cnt The number of parts in which the tree should be partitioned.
     * @returns The component size bounds as a vector.
     *
     * @see calculate_upper_component_weight_bounds()
     */
    template<typename NodeWeight>
        std::vector<NodeWeight> calculate_lower_component_weight_bounds(
                Rational eps, NodeWeight tree_weight, NodeWeight part_cnt);

    /**
     * This class represents the signatures for a tree caclulated by Tree::cut(). 
     * The tree instance MUST outlive the SignaturesForTree instance.
     */
    template<typename Id, typename NodeWeight, typename EdgeWeight>
        struct SignaturesForTree {
            public:
                using Tree = Tree<Id, NodeWeight, EdgeWeight>; /**< The type of the tree with the given template parameters. */
                using Signature = Signature<NodeWeight>; /**< The signature type with the given template parameters. */
                using SignatureMap = SignatureMap<NodeWeight, EdgeWeight>; /**< The type to save the signatures at a node. */

                NodeWeight const part_cnt; /**< The number of parts in the partition. */
                Rational const eps; /**< The approximation parameter. */
                Tree const& tree; /**< The tree for which the signatures were calculated. */
                std::vector<std::vector<SignatureMap>> const signatures; /**< The calculated signatures. */
                std::vector<NodeWeight> const upper_comp_weight_bounds; /**< The upper bounds for the sizes in a signature. */
                std::vector<NodeWeight> const lower_comp_weight_bounds; /**< The lower bounds for the sizes in a signature. */

                /**
                 * Constructor.
                 * @param part_cnt The number of parts in the partition.
                 * @param eps The approximation parameter.
                 * @param tree The tree for which the signatures were calculated.
                 * @param signatures The signatures for \p tree.
                 */
                SignaturesForTree(NodeWeight part_cnt, Rational eps, Tree const& tree, std::vector<std::vector<SignatureMap>> signatures) :
                    part_cnt(part_cnt), eps(eps), tree(tree), signatures(signatures), 
                    upper_comp_weight_bounds(calculate_upper_component_weight_bounds(eps, tree.subtree_weight[0][0], part_cnt)),
                    lower_comp_weight_bounds(calculate_lower_component_weight_bounds(eps, tree.subtree_weight[0][0], part_cnt)) {}

                /** 
                 * A type representing the edges cut by the cutting phase.
                 */
                using CutEdges = std::set<std::pair<Id, Id>>; 

                /**
                 * Calculates the edges which were cut to arrive at \p signature at the root.
                 * @param signature The signature for which the cut edges should be calculated.
                 * @return The edges cut as a set of pairs. One pair specifies the two endpoints of the cut edge.
                 */
                CutEdges cut_edges_for_signature(Signature const& signature) const;

                /**
                 * Calculates the connected components which result in cutting the edges described by \p cut_edges.
                 * @param cut_edges The edges to cut.
                 * @returns A vector of sets. Each set identifies one connected component in the Tree by mapping the
                 *  node ids to the weight of the node.
                 */
                std::vector<std::map<Id, NodeWeight>>
                    components_for_cut_edges(CutEdges const& cut_edges) const;

        };

    /**
     * Prints the signatures to the outputstream.
     * @see std::istream& operator>>(std::istream&, SignaturesForTreeBuilder&) The format used is described here.
     * @param os The outputstream.
     * @param signatures The signatures to print.
     * @returns The outputstream.
     */
    template<typename Id, typename NodeWeight, typename EdgeWeight>
        std::ostream& operator<<(std::ostream& os, SignaturesForTree<Id, NodeWeight, EdgeWeight> const& signatures);


    /**
     * This class is used to build a SignaturesForTree object.
     * We need this class since the members of the SignaturesForTree class are constant.
     */
    template<typename Id=int32_t, typename NodeWeight=int32_t, typename EdgeWeight=int32_t>
        struct SignaturesForTreeBuilder {
            public:
                using Tree = Tree<Id, NodeWeight, EdgeWeight>; /**< The tree type with the given template parameters. */
                using SignatureMap = SignatureMap<NodeWeight, EdgeWeight>; /**< The type to save the signatures at a node */
                
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
                SignaturesForTreeBuilder& with_part_cnt(NodeWeight part_cnt);

                /**
                 * Sets the approximation parameter.
                 * @param eps The approximation parameter
                 * @returns A reference to this.
                 */
                SignaturesForTreeBuilder& with_eps(Rational eps);

                /**
                 * Sets the signatures.
                 * @param signatures The signatures
                 * @returns A reference to this.
                 */
                SignaturesForTreeBuilder& with_signatures(std::vector<std::vector<SignatureMap>> const& signatures);

                /**
                 * Finishes building the signatures.
                 * @returns The finished SignaturesForTree object.
                 */
                SignaturesForTree<Id, NodeWeight, EdgeWeight> finish();

            private:
                NodeWeight part_cnt; /**< The number of parts in the partition. */
                Rational eps; /**< The approximation parameter. */
                std::vector<std::vector<SignatureMap>> signatures; /**< The calculated signatures. */

        };

    /**
     * Populates a SignaturesForTreeBuilder with the values from the inputstream.
     * The format is the following: \n
     * The first line contains three integers \c k \c e_n \c e_d.
     * \c k is the number of parts into which the tree should be partitioned. 
     * \c e_n and \c e_d describe the approximation parameter epsilon where epsilon is equal to
     * <code>e_n/e_d</code>. \n
     * \c n blocks describing the signatures at a node follow 
     * where \c n is the number of nodes in SignaturesForTreeBuilder::tree.
     * The format of a block is the following: \n
     * The first line contains two integers \c i \c sc where \c i is id of the node and \c sc is 
     * the number of different lower frontier sizes at that node.
     * \c sc inner blocks follow each describing all possible signatures for a lower frontier of a
     * specific size. The format for an inner block is the following: \n
     * The first line contains two integers \c s \c c where \c s is the number of nodes in the following 
     * signatures and \c c is the number of signatures. \n
     * \c c lines follow each describing one signature. The signature consists of \c l space-seperate integers
     * where \c l is the length of a signature as given by the parameters \c k and epsilon. At the end of the line
     * is a single integer \c w which describes the cut cost for that signature.
     *
     * You can look at the files with file extension \c signatures in <code>src/test/resources</code> for examples.
     *
     * @param is The inputstream.
     * @param builder The builder.
     * @returns The inputstream.
     */
    template<typename Id, typename NodeWeight, typename EdgeWeight>
        std::istream& operator>>(std::istream& is, SignaturesForTreeBuilder<Id, NodeWeight, EdgeWeight>& builder);
}

// Include template implementation.
#include "Cut.ipp"
#include "CutExtras.ipp"
