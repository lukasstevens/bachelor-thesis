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
 * Almost all structs and functions in this namespace use the template parameters IdType and EdgeWeightType.
 * IdType is the type for node ids and EdgeWeightType is the type with which edge weights of the Tree are stored.
 * @see Node
 * @see Tree
 */
namespace cut {


    /**
     * This class represents a node in the tree.
     * IdType is the type of the node id and EdgeWeightType is the type for storing the edge weights.
     */
    template<typename IdType, typename EdgeWeightType>
        struct Node {
            public:
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

    template<typename SizeType>
        using Signature = std::valarray<SizeType>; /**< The type of a signature. **/

    /**
     * A type which saves the signatures at a node. 
     * It maps the number of nodes in the lower frontier to the possible 
     * signatures with this number of nodes and the signatures are mapped to their cut cost.
     * Since signatures are valarrays, we can use ValarrayHasher and ValarrayEqual.
     * @see ValarrayHasher
     * @see ValarrayEqual
     * @see Signature
     */
    template<typename SizeType, typename EdgeWeightType>
        using SignatureMap = std::vector<std::unordered_map<Signature<SizeType>, EdgeWeightType, 
              valarrutils::ValarrayHasher<SizeType>, valarrutils::ValarrayEqual<SizeType>>>;

    /**
     * This saves the signatures which were used to arrive the current signature.
     * @see SignatureMapWithPrev
     */
    template<typename SizeType>
        struct PreviousSignatures {
            using Signature = Signature<SizeType>;
            std::pair<SizeType, Signature> left_sibling_sig;
            std::pair<SizeType, Signature> right_child_sig;
            bool was_parent_edge_cut;

            PreviousSignatures() = default;

            PreviousSignatures(
                    std::pair<SizeType, Signature> left_sibling_sig,
                    std::pair<SizeType, Signature> right_child_sig,
                    bool was_parent_edge_cut) :
                left_sibling_sig(left_sibling_sig), right_child_sig(right_child_sig), 
                was_parent_edge_cut(was_parent_edge_cut) {}
        };

    /**
     * A type which saves the signatures at a node and also the previous signature for each signature. 
     * This is used in cut_edges_for_signature().
     * @see Tree::SignatureMap
     */
    template<typename SizeType, typename EdgeWeightType>
        using SignatureMapWithPrev = std::map<SizeType, 
              std::unordered_map<Signature<SizeType>, std::pair<EdgeWeightType, PreviousSignatures<SizeType>>,
              valarrutils::ValarrayHasher<SizeType>, valarrutils::ValarrayEqual<SizeType>>>;

    using RationalType = mpq_class; /**< The type of a rational. **/

    template<typename Idtype, typename EdgeWeightType>
        struct SignaturesForTree;

    /**
     * This class represents a tree. The nodes of a tree are arranged in levels.
     * The nodes contain the necessary information to determine the parent and the
     * children of a node in the level above and below respectively.
     * For an explanation of the types 
     * @see Node
     */
    template<typename IdType, typename EdgeWeightType>
        struct Tree {
            public:
                using SizeType = IdType; /**< SizeType is the type for counting nodes. It is the same as IdType. */
                using Node = Node<IdType, EdgeWeightType>; /**< Type of a Node according to template parameters */
                using Signature = Signature<SizeType>; /**< Type of a signature. */
                using SignatureMap = SignatureMap<SizeType, EdgeWeightType>; /**< The type to save the signatures at a node */
                /** Similar to SignatureMap only with information about previous signatures. */
                using SignatureMapWithPrev = SignatureMapWithPrev<SizeType, EdgeWeightType>;

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
                static Tree build_tree(std::map<IdType, std::map<IdType, EdgeWeightType>> const& tree, IdType root_id);

                /**
                 * Similiar to the overloaded function. Uses a random root. 
                 * For this to work the tree must be undirected.
                 * @param tree The tree to use.
                 * @returns the tree built from \p tree.
                 *
                 * @see build_tree()
                 */
                static Tree build_tree(std::map<IdType, std::map<IdType, EdgeWeightType>> const& tree);

                /**
                 * Calculates the sizes of the subtrees in the tree.
                 * It stores the sizes in the member Tree::tree_sizes.
                 */
                void calculate_subtree_sizes();


                /**
                 * Cuts the tree with the given parameters.
                 * @param eps The approximation factor to use.
                 * @param part_cnt The number of parts in which the tree should be partitioned.
                 * @returns The signatures calculated for the given parameters.
                 *
                 * @see SignaturesForTree
                 */
                SignaturesForTree<IdType, EdgeWeightType> cut(RationalType eps, SizeType part_cnt);

                /**
                 * Calculates the signatures of the tree with information about the previous signatures.
                 * Additionally no signatures greater than \p signature are allowed.
                 * Analogous to Tree::cut().
                 */
                std::vector<std::vector<SignatureMapWithPrev>> 
                    cut_with_prev(RationalType eps, SizeType part_cnt, Signature const& signature) const;

                /**
                 * Calculates the node_idx for the node with the id \p node_id.
                 * @param node_id The id of the node.
                 * @returns A pair describing the index of the node in the levels of the tree.
                 * Returns <code>(std::numeric_limits<size_t>::max(), std::numeric_limits<size_t>::max())</code>
                 * if no node with id \p node_id exists.
                 */
                std::pair<size_t, size_t> get_node_idx(IdType node_id) const;

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
                 * @param comp_size_bounds The upper component size bounds(exclusive) for the signature.
                 */
                static SignatureMap cut_at_node(
                        Node const& node, 
                        SizeType node_subtree_size,
                        SizeType left_siblings_size,
                        SignatureMap const& left_sibling_sigs, 
                        SignatureMap const& right_child_sigs, 
                        std::vector<SizeType> const& comp_size_bounds);


                /**
                 * Calculates the signatures at a node.
                 * This works analogous to Tree::cut_at_node(), only that the previous signatures are calculated
                 * and no signatures greater than \p signature are allowed.
                 * @param node The current node.
                 * @param subtree_size The size of the subtree rooted at the current node.
                 * @param left_sibling_sigs The signatures at the left sibling.
                 * @param right_child_sigs The signatures at the right child.
                 * @param comp_size_bounds The upper component size bounds(exclusive) for the signature.
                 * @param The maximum allowed signature.
                 */
                static SignatureMapWithPrev cut_at_node_with_prev(
                        Node const& node, 
                        SizeType subtree_size,
                        SignatureMapWithPrev const& left_sibling_sigs, 
                        SignatureMapWithPrev const& right_child_sigs,
                        std::vector<SizeType> const& comp_size_bounds,
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
    template<typename IdType, typename EdgeWeightType>
        std::istream& operator>>(std::istream& is, Tree<IdType, EdgeWeightType>& tree);

    /**
     * Prints a Tree to the outputstream.
     * @see std::istream& operator>>(std::istream&, Tree&) The format for outputting a tree.
     * @param os The outputstream.
     * @param tree The tree to print.
     * @returns The outputstream.
     */
    template<typename IdType, typename EdgeWeightType>
        std::ostream& operator<<(std::ostream& os, Tree<IdType, EdgeWeightType> const& tree);

    /**
     * Calculates the upper (exclusive) bounds on the component sizes for each index of a signature.
     * @param eps The approximation factor.
     * @param node_cnt The number of nodes in the tree.
     * @param part_cnt The number of parts in which the tree should be partitioned.
     * @returns The component size bounds as a vector.
     *
     * @see calculate_lower_component_size_bounds()
     */
    template<typename SizeType>
        std::vector<SizeType> calculate_upper_component_size_bounds(RationalType eps, SizeType node_cnt, SizeType part_cnt);

    /**
     * Calculates the lower (inclusive) bounds on the component sizes for each index of a signature.
     * @param eps The approximation factor.
     * @param node_cnt The number of nodes in the tree.
     * @param part_cnt The number of parts in which the tree should be partitioned.
     * @returns The component size bounds as a vector.
     *
     * @see calculate_upper_component_size_bounds()
     */
    template<typename SizeType>
        std::vector<SizeType> calculate_lower_component_size_bounds(RationalType eps, SizeType node_cnt, SizeType part_cnt);

    /**
     * This class represents the signatures for a tree caclulated by Tree::cut(). 
     * The tree instance MUST outlive the SignaturesForTree instance.
     */
    template<typename IdType, typename EdgeWeightType>
        struct SignaturesForTree {
            public:
                using SizeType = IdType; /**< A type to count the nodes of the tree. */
                using Tree = Tree<IdType, EdgeWeightType>; /**< The type of the tree with the given template parameters. */
                using Signature = Signature<SizeType>; /**< The signature type with the given template parameters. */
                using SignatureMap = SignatureMap<IdType, EdgeWeightType>; /**< The type to save the signatures at a node. */

                SizeType const part_cnt; /**< The number of parts in the partition. */
                RationalType const eps; /**< The approximation parameter. */
                Tree const& tree; /**< The tree for which the signatures were calculated. */
                std::vector<std::vector<SignatureMap>> const signatures; /**< The calculated signatures. */
                std::vector<SizeType> const upper_comp_size_bounds; /**< The upper bounds for the sizes in a signature. */
                std::vector<SizeType> const lower_comp_size_bounds; /**< The lower bounds for the sizes in a signature. */

                /**
                 * Constructor.
                 * @param part_cnt The number of parts in the partition.
                 * @param eps The approximation parameter.
                 * @param tree The tree for which the signatures were calculated.
                 * @param signatures The signatures for \p tree.
                 */
                SignaturesForTree(SizeType part_cnt, RationalType eps, Tree const& tree, std::vector<std::vector<SignatureMap>> signatures) :
                    part_cnt(part_cnt), eps(eps), tree(tree), signatures(signatures), 
                    upper_comp_size_bounds(calculate_upper_component_size_bounds(eps, tree.tree_sizes[0][0], part_cnt)),
                    lower_comp_size_bounds(calculate_lower_component_size_bounds(eps, tree.tree_sizes[0][0], part_cnt)) {}

                /** 
                 * A type representing the edges cut by the cutting phase.
                 */
                using CutEdges = std::set<std::pair<IdType, IdType>>; 

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
                std::vector<std::set<IdType>> components_for_cut_edges(CutEdges const& cut_edges) const;

        };

    /**
     * Prints the signatures to the outputstream.
     * @see std::istream& operator>>(std::istream&, SignaturesForTreeBuilder&) The format used is described here.
     * @param os The outputstream.
     * @param signatures The signatures to print.
     * @returns The outputstream.
     */
    template<typename IdType, typename EdgeWeightType>
        std::ostream& operator<<(std::ostream& os, SignaturesForTree<IdType, EdgeWeightType> const& signatures);


    /**
     * This class is used to build a SignaturesForTree object.
     * We need this class since the members of the SignaturesForTree class are constant.
     */
    template<typename IdType, typename EdgeWeightType>
        struct SignaturesForTreeBuilder {
            public:
                using Tree = Tree<IdType, EdgeWeightType>; /**< The tree type with the given template parameters. */
                using SizeType = IdType; /**< The type to count nodes of the tree */
                using SignatureMap = SignatureMap<SizeType, EdgeWeightType>; /**< The type to save the signatures at a node */
                
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
                SignaturesForTreeBuilder& with_signatures(std::vector<std::vector<SignatureMap>> const& signatures);

                /**
                 * Finishes building the signatures.
                 * @returns The finished SignaturesForTree object.
                 */
                SignaturesForTree<IdType, EdgeWeightType> finish();

            private:
                SizeType part_cnt; /**< The number of parts in the partition. */
                RationalType eps; /**< The approximation parameter. */
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
    template<typename IdType, typename EdgeWeightType>
        std::istream& operator>>(std::istream& is, SignaturesForTreeBuilder<IdType, EdgeWeightType>& builder);
}

namespace cut {

    template<typename IdType, typename EdgeWeightType>
        Node<IdType, EdgeWeightType>::Node(IdType id, EdgeWeightType parent_edge_weight, 
                size_t parent_idx, std::pair<size_t, size_t> children_idx_range) : 
            id(id), parent_edge_weight(parent_edge_weight), parent_idx(parent_idx), children_idx_range(children_idx_range) {}


    template<typename IdType, typename EdgeWeightType>
        Tree<IdType, EdgeWeightType> Tree<IdType, EdgeWeightType>::build_tree(std::map<IdType, std::map<IdType, EdgeWeightType>> const& tree_map, IdType root_id) {
            Tree tree;
            // Use a struct to represent an incomplete node since the child_idx_range is not known.
            struct NodeStub {
                IdType const id; 
                EdgeWeightType const parent_edge_weight;
                size_t const parent_idx;
                bool const has_left_sibling;
                size_t const level;

                NodeStub(IdType id, EdgeWeightType p_e_w, size_t p_i, bool h_l_s, size_t lvl) :
                    id(id), parent_edge_weight(p_e_w), parent_idx(p_i), has_left_sibling(h_l_s), level(lvl) {}
            };

            // Do a BFS to build the tree.
            std::list<NodeStub> queue;
            queue.push_back(NodeStub(root_id, 0, 0, false, 0));
            size_t next_child_idx = 0;
            while(!queue.empty()) {
                NodeStub curr_node = queue.front();
                queue.pop_front();

                // Add a new level to tree if the BFS reaches a new level and reset the child_idx.
                if (curr_node.level == tree.levels.size()) {
                    tree.levels.push_back(std::vector<Node>());
                    tree.has_left_sibling.push_back(std::vector<bool>());
                    next_child_idx = 0;
                }

                // Save the index of the first child.
                size_t old_next_child_idx = next_child_idx;
                if (tree_map.find(curr_node.id) != tree_map.cend()) {
                    for (auto const neighbor : tree_map.at(curr_node.id)) {
                        // Check if neighbor is the parent.
                        if (curr_node.level == 0 || neighbor.first != tree.levels[curr_node.level - 1][curr_node.parent_idx].id) {
                            bool curr_has_left_sibling = !(old_next_child_idx == next_child_idx);
                            queue.emplace_back(neighbor.first, neighbor.second, tree.levels[curr_node.level].size(), curr_has_left_sibling, curr_node.level + 1); 
                            ++next_child_idx;
                        }
                    }
                }

                tree.levels[curr_node.level].emplace_back(curr_node.id, curr_node.parent_edge_weight, 
                        curr_node.parent_idx, std::make_pair(old_next_child_idx, next_child_idx));
                tree.has_left_sibling[curr_node.level].push_back(curr_node.has_left_sibling);

            }

            tree.calculate_subtree_sizes();

            return tree;
        }

    template<typename IdType, typename EdgeWeightType>
        void Tree<IdType, EdgeWeightType>::calculate_subtree_sizes() {
            Tree& tree = *this;
            for (auto const& lvl : tree.levels) {
                tree.tree_sizes.emplace_back(lvl.size(), 1);
            }

            for (size_t lvl_idx = tree.levels.size() - 2; lvl_idx < tree.levels.size(); --lvl_idx) {
                size_t node_idx = 0;
                for (auto const& node : tree.levels[lvl_idx]) {
                    for (size_t child_idx = node.children_idx_range.first; child_idx < node.children_idx_range.second; ++child_idx) {
                        tree.tree_sizes[lvl_idx][node_idx] += tree.tree_sizes[lvl_idx + 1][child_idx];
                    }
                    ++node_idx;
                }
            }
        }

    template<typename IdType, typename EdgeWeightType>
        Tree<IdType, EdgeWeightType> Tree<IdType, EdgeWeightType>::build_tree(std::map<IdType, std::map<IdType, EdgeWeightType>> const& tree_map) {
            return build_tree(tree_map, tree_map.begin()->first);
        }

    template<typename IdType, typename EdgeWeightType>
        SignatureMap<IdType, EdgeWeightType> Tree<IdType, EdgeWeightType>::cut_at_node(
                Node const& node, 
                SizeType node_subtree_size,
                SizeType left_siblings_size,
                SignatureMap const& left_sibling_sigs, 
                SignatureMap const& right_child_sigs, 
                std::vector<SizeType> const& comp_size_bounds) {

            // The maximum amount of nodes in a Signature at the current node is the frontier size
            // which is left_sibling_size + node_subtree_size plus 1
            SignatureMap node_sigs(static_cast<size_t>(left_siblings_size + node_subtree_size) + 1);

            // Iterate over all calculated signatures of the left sibling and the rightmost child according
            // to the dynamic programming scheme described in the paper FF13.
            for (SizeType left_sibling_node_cnt = 0; static_cast<size_t>(left_sibling_node_cnt) < left_sibling_sigs.size(); ++left_sibling_node_cnt) {
                auto const& left_sibling_sigs_with_node_cnt = left_sibling_sigs[static_cast<size_t>(left_sibling_node_cnt)];

                for (SizeType child_node_cnt = 0; static_cast<size_t>(child_node_cnt) < right_child_sigs.size(); ++child_node_cnt) {
                    auto const& child_sigs_with_node_cnt = right_child_sigs[static_cast<size_t>(child_node_cnt)];

                    for (auto const& left_sibling_sig : left_sibling_sigs_with_node_cnt) {
                        for (auto const& child_sig : child_sigs_with_node_cnt) {
                            // First case: The edge from the current node to its parent is not cut.
                            SizeType frontier_size = left_sibling_node_cnt + child_node_cnt;
                            size_t frontier_size_size_t = static_cast<size_t>(frontier_size);
                            EdgeWeightType cut_cost = left_sibling_sig.second + child_sig.second;
                            Signature sig = left_sibling_sig.first + child_sig.first;

                            auto prev_cut_cost_it = node_sigs[frontier_size_size_t].find(sig);
                            if (prev_cut_cost_it == node_sigs[frontier_size_size_t].end()
                                    || cut_cost < prev_cut_cost_it->second) {
                                node_sigs[frontier_size_size_t][sig] = cut_cost;
                            }

                            // Second case: The edge from the current node to its parent is cut.
                            SizeType const node_comp_size = node_subtree_size - child_node_cnt;
                            // Check if the current size of the component which includes the current node is smaller than
                            // the maximum allowed size.
                            if (node_comp_size >= comp_size_bounds.back()) {
                                continue;
                            } else {
                                frontier_size += node_comp_size;
                                frontier_size_size_t = static_cast<size_t>(frontier_size);
                                cut_cost += node.parent_edge_weight;

                                // Adjust the signature to account for the component which contains the current node.
                                size_t i = 0; 
                                while (node_comp_size >= comp_size_bounds[i]) { ++i; }
                                sig[i] += 1;

                                prev_cut_cost_it = node_sigs[frontier_size_size_t].find(sig);
                                if (prev_cut_cost_it == node_sigs[frontier_size_size_t].end()
                                        || cut_cost < prev_cut_cost_it->second) {
                                    node_sigs[frontier_size_size_t][sig] = cut_cost;
                                }
                            }
                        }
                    }
                }
            }
            return node_sigs;
        }


    template<typename IdType, typename EdgeWeightType>
        SignaturesForTree<IdType, EdgeWeightType> Tree<IdType, EdgeWeightType>::cut(RationalType eps, SizeType part_cnt) {
            std::vector<std::vector<SignatureMap>> signatures;
            for (auto const& lvl : this->levels) {
                signatures.emplace_back(lvl.size());
            }

            // Calculate the size intervals of the connected components of a signature.
            std::vector<SizeType> const comp_size_bounds = calculate_upper_component_size_bounds(eps, this->tree_sizes[0][0], part_cnt);

            // Iterate over all nodes except the root starting with the node one the bottom left.
            for (size_t lvl_idx = this->levels.size() - 1; lvl_idx > 0; --lvl_idx) {
                SizeType left_siblings_size = 0; 
                for (size_t node_idx = 0; node_idx < this->levels[lvl_idx].size(); ++node_idx) {
                    Node const& node = this->levels[lvl_idx][node_idx];
                    SizeType const node_subtree_size = this->tree_sizes[lvl_idx][node_idx];
                    bool const node_has_left_sibling = this->has_left_sibling[lvl_idx][node_idx];
                    bool const node_has_child = node.children_idx_range.first < node.children_idx_range.second;

                    // The signature which contains 0 nodes, is the 0-vector and has 0 cut cost is
                    // always present, even if the node does not exist.
                    SignatureMap const empty_map({{{{Signature(comp_size_bounds.size()), 0}}}});

                    SignatureMap const* left_sibling_sigs = &empty_map;
                    SignatureMap const* child_sigs = &empty_map;

                    // Adjust the reference to the signatures if the node has a left sibling or
                    // has a child respectively.
                    // TODO: Think about optimizing the case where there is no left sibling or there is no child.
                    // At the moment we are adding the 0-vector to all signatures calculated beforehand which is 
                    // unnecessary.
                    if (node_has_left_sibling) {
                        left_sibling_sigs = &signatures[lvl_idx][node_idx - 1];
                    }
                    if (node_has_child) {
                        child_sigs = &signatures[lvl_idx + 1][node.children_idx_range.second - 1];
                    }


                    signatures[lvl_idx][node_idx] = cut_at_node(node, node_subtree_size, left_siblings_size,
                            *left_sibling_sigs, *child_sigs, comp_size_bounds);
                    left_siblings_size += node_subtree_size;
                }
            }

            // Calculate the signatures at the root according to the paper FF13. 
            // Signatures which contain less then the total amount of nodes are ignored.
            SizeType const node_cnt = this->tree_sizes[0][0];
            size_t const node_cnt_size_t = static_cast<size_t>(node_cnt);
            SignatureMap& root_sigs = signatures[0][0];
            root_sigs = SignatureMap(node_cnt_size_t + 1);
            SignatureMap& child_sigs = signatures[1].back();

            for (SizeType root_comp_node_cnt = comp_size_bounds.back() - 1; root_comp_node_cnt > 0; --root_comp_node_cnt) {
                for (auto const& sig : child_sigs[static_cast<size_t>(node_cnt - root_comp_node_cnt)]) {
                    Signature root_sig(sig.first);
                    size_t i = 0;
                    while(root_comp_node_cnt >= comp_size_bounds[i]) { ++i; }
                    root_sig[i] += 1;

                    auto prev_cut_cost_it = root_sigs[node_cnt_size_t].find(root_sig);
                    if (prev_cut_cost_it == root_sigs[node_cnt_size_t].end()
                            || sig.second < prev_cut_cost_it->second) {
                        root_sigs[node_cnt_size_t][root_sig] = sig.second;
                    }
                }
            }

            return SignaturesForTree<IdType, EdgeWeightType>(part_cnt, eps, *this, signatures);
        }

    template<typename IdType, typename EdgeWeightType>
       SignatureMapWithPrev<IdType, EdgeWeightType> Tree<IdType, EdgeWeightType>::cut_at_node_with_prev(
                Node const& node, 
                SizeType subtree_size,
                SignatureMapWithPrev const& left_sibling_sigs, 
                SignatureMapWithPrev const& right_child_sigs,
                std::vector<SizeType> const& upper_comp_size_bounds,
                Signature const& signature
                ) {

            // Iterate over all calculated signatures of the left sibling and the rightmost child according
            // to the dynamic programming scheme described in the paper FF13.
            SignatureMapWithPrev node_sigs;
            for (auto const& left_sibling_sigs_with_size : left_sibling_sigs) {
                for (auto const& child_sigs_with_size : right_child_sigs) {
                    for (auto const& left_sibling_sig : left_sibling_sigs_with_size.second) {
                        for (auto const& child_sig : child_sigs_with_size.second) {
                            // First case: The edge from the current node to its parent is not cut.
                            SizeType frontier_size = left_sibling_sigs_with_size.first + child_sigs_with_size.first;

                            // The entries in SignatureMapWithPrev are pairs of the cut cost and the previous signatures. 
                            EdgeWeightType left_sibling_sig_cut_cost = left_sibling_sig.second.first;
                            EdgeWeightType child_sig_cut_cost = child_sig.second.first;

                            EdgeWeightType cut_cost = left_sibling_sig_cut_cost + child_sig_cut_cost;
                            Signature node_sig = left_sibling_sig.first + child_sig.first;

                            // Check if the current signature is feasible.
                            if (!((node_sig <= signature).min())) {
                                continue;
                            }

                            PreviousSignatures<IdType> previous_signatures(
                                    std::make_pair(left_sibling_sigs_with_size.first, left_sibling_sig.first),
                                    std::make_pair(child_sigs_with_size.first, child_sig.first),
                                    false);

                            if (node_sigs[frontier_size].find(node_sig) == node_sigs[frontier_size].end()
                                    || cut_cost < node_sigs[frontier_size][node_sig].first) {
                                node_sigs[frontier_size][node_sig] = std::make_pair(cut_cost, previous_signatures);
                            } 

                            // Second case: The edge from the current node to its parent is cut.
                            // Check if the current size of the component which includes the current node is smaller than
                            // the maximum allowed size.
                            SizeType const node_comp_size = subtree_size - child_sigs_with_size.first;
                            if (node_comp_size >= upper_comp_size_bounds.back()) {
                                continue;
                            } else {
                                frontier_size += node_comp_size;
                                cut_cost += node.parent_edge_weight;

                                // Adjust the signature to account for the component which contains the current node.
                                size_t i = 0; 
                                while (node_comp_size >= upper_comp_size_bounds[i]) { ++i; }
                                node_sig[i] += 1;
                                // Check if the current signature is feasible.
                                if (!((node_sig <= signature).min())) {
                                    continue;
                                }

                                previous_signatures.was_parent_edge_cut = true;
                                if (node_sigs[frontier_size].find(node_sig) == node_sigs[frontier_size].end()
                                        || cut_cost < node_sigs[frontier_size][node_sig].first) {
                                    node_sigs[frontier_size][node_sig] = std::make_pair(cut_cost, previous_signatures);
                                } 
                            }
                        }
                    }
                }
            }
            return node_sigs;
        }

    template<typename IdType, typename EdgeWeightType>
        std::vector<std::vector<SignatureMapWithPrev<IdType, EdgeWeightType>>> 
        Tree<IdType, EdgeWeightType>::cut_with_prev(RationalType eps, SizeType part_cnt, Signature const& signature) const {

            auto const upper_comp_size_bounds = calculate_upper_component_size_bounds(eps, this->tree_sizes[0][0], part_cnt);
            std::vector<std::vector<SignatureMapWithPrev>> signatures_with_prev;
            for (auto const& lvl : this->levels) {
                signatures_with_prev.emplace_back(lvl.size());
            }

            // Iterate over all nodes except the root starting with the node one the bottom left.
            for (size_t lvl_idx = this->levels.size() - 1; lvl_idx > 0; --lvl_idx) {
                for (size_t node_idx = 0; node_idx < this->levels[lvl_idx].size(); ++node_idx) {
                    Node const& node = this->levels[lvl_idx][node_idx];
                    SizeType const node_subtree_size = this->tree_sizes[lvl_idx][node_idx];
                    SignatureMapWithPrev empty_map;
                    // The only signature which always has cut value smaller infinity(even if the node does not exist) is the 0-vector.
                    Signature const empty_signature = Signature(signature.size());
                    std::pair<SizeType, Signature> empty_prev_signature(0, empty_signature);
                    empty_map[0][empty_signature] = std::make_pair(0, 
                            PreviousSignatures<IdType>(empty_prev_signature, empty_prev_signature, false));

                    SignatureMapWithPrev const* left_sibling_sigs = &empty_map;
                    SignatureMapWithPrev const* child_sigs = &empty_map;

                    // Adjust the reference to the signatures if the node has a left sibling or
                    // has a child respectively.
                    bool const node_has_left_sibling = this->has_left_sibling[lvl_idx][node_idx];
                    bool const node_has_child = node.children_idx_range.first < node.children_idx_range.second;
                    if (node_has_left_sibling) {
                        left_sibling_sigs = &signatures_with_prev[lvl_idx][node_idx - 1];
                    }
                    if (node_has_child) {
                        child_sigs = &signatures_with_prev[lvl_idx + 1][node.children_idx_range.second - 1];
                    }

                    signatures_with_prev[lvl_idx][node_idx] = 
                        cut_at_node_with_prev(node, node_subtree_size, *left_sibling_sigs, *child_sigs, 
                                upper_comp_size_bounds, signature);

                }

                // Calculate the signatures at the root according to the paper FF13. 
                // Signatures which contain less then the total amount of nodes are ignored.
                SignatureMapWithPrev& root_sigs = signatures_with_prev[0][0];
                SizeType const node_cnt = this->tree_sizes[0][0];
                for (auto const& child_sigs_with_size : signatures_with_prev[1].back()) {
                    SizeType const root_comp_size = node_cnt - child_sigs_with_size.first;
                    if (root_comp_size >= upper_comp_size_bounds.back()) {
                        continue;
                    } else {
                        for (auto const& child_sig : child_sigs_with_size.second) {

                            // Initialize root_sig equal to child_sig and then adjust for the size
                            // of the component which contians the root.
                            Signature root_sig(child_sig.first);
                            size_t i = 0;
                            while(root_comp_size >= upper_comp_size_bounds[i]) { ++i; }
                            root_sig[i] += 1;

                            EdgeWeightType cut_cost = child_sig.second.first;

                            PreviousSignatures<IdType> previous_signatures(
                                    std::make_pair(0, Signature(signature.size())),
                                    std::make_pair(child_sigs_with_size.first, child_sig.first),
                                    false);

                            if (root_sigs[node_cnt].find(root_sig) == root_sigs[node_cnt].end() || 
                                    cut_cost < root_sigs[node_cnt][root_sig].first) {
                                root_sigs[node_cnt][root_sig] = std::make_pair(cut_cost, previous_signatures);
                            }
                        }
                    }
                }
            }

            return signatures_with_prev;
        }

    template<typename IdType, typename EdgeWeightType>
        std::pair<size_t, size_t> Tree<IdType, EdgeWeightType>::get_node_idx(IdType node_id) const {
            for (size_t lvl_idx = 0; lvl_idx < this->levels.size(); ++lvl_idx) {
                for (size_t node_idx = 0; node_idx < this->levels[lvl_idx].size(); ++node_idx) {
                    if (this->levels[lvl_idx][node_idx].id == node_id) {
                        return std::make_pair(lvl_idx, node_idx);
                    }
                }
            }
            return std::make_pair(std::numeric_limits<size_t>::max(), 
                    std::numeric_limits<size_t>::max());
        }

    template<typename IdType, typename EdgeWeightType>
        std::string Tree<IdType, EdgeWeightType>::as_graphviz() const {

            std::stringstream stream;
            stream << "digraph tree {\n";
            for (size_t lvl_idx = this->levels.size() - 1; lvl_idx > 0; --lvl_idx) {
                for (size_t node_idx = 0; node_idx < this->levels[lvl_idx].size(); ++node_idx) {
                    Node const& node = this->levels[lvl_idx][node_idx];
                    Node const& parent = this->levels[lvl_idx - 1][node.parent_idx];
                    stream << "\t" << parent.id;
                    stream << " -> " << node.id;
                    stream << "[label=\"" << node.parent_edge_weight << "\"";
                    stream << "]\n";
                }
            }

            int32_t invis_node = -1;
            for (size_t lvl_idx = this->levels.size() - 1; lvl_idx > 0; --lvl_idx) {
                std::vector<Node> const& lvl = this->levels[lvl_idx];
                std::stringstream node_ordering; 
                node_ordering << "{rank=same " << lvl[0].id;
                for (size_t node_idx = 1; node_idx < this->levels[lvl_idx].size(); ++node_idx) {
                    Node const& node = lvl[node_idx];
                    stream << "\t" << invis_node << "[label=\"\", width=0.1, style=invis]\n";
                    stream << "\t" << this->levels[lvl_idx - 1][node.parent_idx].id;
                    stream << " -> " << invis_node << "[style=invis]\n";
                    node_ordering << " -> " << invis_node << " -> " << node.id;
                    --invis_node;
                }
                if (lvl.size() > 1) {
                    node_ordering << "[style=invis]";
                }
                node_ordering << "}\n";
                stream << node_ordering.str();
            }

            stream << "}\n";
            return stream.str();
        }

    template<typename IdType, typename EdgeWeightType>
        std::istream& operator>>(std::istream& is, Tree<IdType, EdgeWeightType>& tree) {
            using SizeType = IdType;
            SizeType node_cnt;
            IdType root_id;
            is >> node_cnt >> root_id;

            std::map<IdType, std::map<IdType, EdgeWeightType>> tree_map;
            for (SizeType edge_idx = 0; edge_idx < node_cnt - 1; ++edge_idx) {
                IdType from; 
                IdType to; 
                EdgeWeightType weight;
                is >> from >> to >> weight;
                tree_map[from][to] = weight;
            }
            tree = Tree<IdType, EdgeWeightType>::build_tree(tree_map, root_id);

            return is;
        }

    template<typename IdType, typename EdgeWeightType>
        std::ostream& operator<<(std::ostream& os, Tree<IdType, EdgeWeightType> const& tree) {
            os << tree.tree_sizes[0][0] << " " << tree.levels[0][0].id << std::endl;
            for (size_t lvl_idx = 1; lvl_idx < tree.levels.size(); ++lvl_idx) {
                for (auto const& node : tree.levels[lvl_idx]) {
                    os << tree.levels[lvl_idx - 1][node.parent_idx].id << " ";
                    os << node.id << " " << node.parent_edge_weight << std::endl;
                }
            }
            return os;
        }

    template<typename SizeType>
        std::vector<SizeType> calculate_upper_component_size_bounds(RationalType eps, SizeType node_cnt, SizeType part_cnt) {
            using Rational = RationalType;

            // Calculate the sizes of the components in a signature according to the paper FF13.
            // We use rationals here to prevent numerical instabilities.
            std::vector<SizeType> comp_sizes;
            Rational n_div_k = Rational(gmputils::ceil_to_int<SizeType>(Rational(node_cnt, part_cnt)));
            Rational curr_upper_bound = eps * n_div_k;
            Rational upper_bound = (Rational(1) + eps) * n_div_k;
            while (curr_upper_bound < upper_bound) {
                comp_sizes.push_back(gmputils::ceil_to_int<SizeType>(curr_upper_bound));
                curr_upper_bound *= (Rational(1) + eps);
            }
            comp_sizes.push_back(gmputils::floor_to_int<SizeType>(upper_bound + Rational(1)));
            return comp_sizes;
        }

    template<typename SizeType>
        std::vector<SizeType> calculate_lower_component_size_bounds(RationalType eps, SizeType node_cnt, SizeType part_cnt) {
            std::vector<SizeType> const upper_comp_size_bounds = calculate_upper_component_size_bounds(eps, node_cnt, part_cnt);
            std::vector<SizeType> lower_comp_size_bounds({1});
            lower_comp_size_bounds.insert(lower_comp_size_bounds.end(), upper_comp_size_bounds.begin(), upper_comp_size_bounds.end() - 1);

            return lower_comp_size_bounds;
        }


    template<typename IdType, typename EdgeWeightType>
        typename SignaturesForTree<IdType, EdgeWeightType>::CutEdges 
        SignaturesForTree<IdType, EdgeWeightType>::cut_edges_for_signature(Signature const& signature) const {

            std::vector<std::vector<SignatureMapWithPrev<IdType, EdgeWeightType>>> signatures_with_prev 
                = this->tree.cut_with_prev(this->eps, this->part_cnt, signature);
            CutEdges cut_edges;

            struct SignatureAtNode {
                std::pair<SizeType, Signature> sig_with_size;
                std::pair<size_t, size_t> node_idx;

                SignatureAtNode(std::pair<SizeType, Signature> sig_with_size, std::pair<size_t, size_t> node_idx) :
                    sig_with_size(sig_with_size), node_idx(node_idx) {}
            };

            std::list<SignatureAtNode> queue; 
            queue.emplace_back(std::make_pair(this->tree.tree_sizes[0][0], signature), std::make_pair(0, 0));
            while (!queue.empty()) {
                SignatureAtNode const sig_at_node = queue.front();
                queue.pop_front();

                Node<IdType, EdgeWeightType> const& node = this->tree.levels[sig_at_node.node_idx.first][sig_at_node.node_idx.second];
                auto const& node_idx = sig_at_node.node_idx;

                SignatureMapWithPrev<IdType, EdgeWeightType> const& signatures_with_prev_at_node = 
                    signatures_with_prev[node_idx.first][node_idx.second];
                PreviousSignatures<IdType> const& previous_signatures = signatures_with_prev_at_node
                    .at(sig_at_node.sig_with_size.first).at(sig_at_node.sig_with_size.second).second;

                bool const node_has_left_sibling = this->tree.has_left_sibling[node_idx.first][node_idx.second];
                bool const node_has_child = node.children_idx_range.first < node.children_idx_range.second;
                if (node_has_left_sibling) {
                    auto left_sibling_idx(sig_at_node.node_idx);
                    left_sibling_idx.second -= 1;
                    queue.emplace_back(
                            previous_signatures.left_sibling_sig,
                            left_sibling_idx);
                }

                if (node_has_child) {
                    auto right_child_idx(sig_at_node.node_idx);
                    right_child_idx.first += 1;
                    right_child_idx.second = node.children_idx_range.second - 1;
                    queue.emplace_back(
                            previous_signatures.right_child_sig,
                            right_child_idx);
                }

                if (previous_signatures.was_parent_edge_cut) {
                    Node<IdType, EdgeWeightType> const& parent = this->tree.levels[sig_at_node.node_idx.first - 1][node.parent_idx];
                    cut_edges.emplace(node.id, parent.id);
                }
            }

            return cut_edges;
        }

    template<typename IdType, typename EdgeWeightType>
        std::vector<std::set<IdType>> SignaturesForTree<IdType, EdgeWeightType>::components_for_cut_edges(CutEdges const& cut_edges) const {
            std::vector<std::set<IdType>> components;

            struct NodeInfo {
                std::pair<size_t, size_t> node_idx;
                size_t component_idx;

                NodeInfo(std::pair<size_t, size_t> node_idx, size_t component_idx) :
                    node_idx(node_idx), component_idx(component_idx) {}
            };

            components.emplace_back();
            std::list<NodeInfo> queue;

            queue.emplace_back(std::make_pair(0, 0), 0);

            while (!queue.empty()) {
                auto curr_info = queue.front();
                Node<IdType, EdgeWeightType> const& curr_node = this->tree.levels[curr_info.node_idx.first][curr_info.node_idx.second];
                queue.pop_front();

                components[curr_info.component_idx].insert(curr_node.id);
                for (size_t child_idx = curr_node.children_idx_range.first; 
                        child_idx < curr_node.children_idx_range.second; ++child_idx) {
                    Node<IdType, EdgeWeightType> const& child = this->tree.levels[curr_info.node_idx.first + 1][child_idx];
                    std::pair<size_t, size_t> const child_node_idx(curr_info.node_idx.first + 1, child_idx);
                    if (cut_edges.find(std::make_pair(child.id, curr_node.id)) != cut_edges.end() || 
                            cut_edges.find(std::make_pair(curr_node.id, child.id)) != cut_edges.end()) {
                        components.emplace_back();
                        queue.emplace_back(child_node_idx, components.size() - 1);
                    } else {
                        queue.emplace_back(child_node_idx, curr_info.component_idx);
                    }
                }
            }

            return components;
        }

    template<typename IdType, typename EdgeWeightType>
        std::ostream& operator<<(std::ostream& os, SignaturesForTree<IdType, EdgeWeightType> const& signatures) {
            os << signatures.part_cnt << " ";  
            os << signatures.eps.get_num() << " " << signatures.eps.get_den() << std::endl;
            os << std::endl;

            auto const& sigs = signatures.signatures;
            for (size_t lvl_idx = 0; lvl_idx < sigs.size(); ++lvl_idx) {
                for (size_t node_idx = 0; node_idx < sigs[lvl_idx].size(); ++node_idx) {
                    auto const& node_sigs = sigs[lvl_idx][node_idx];
                    os << signatures.tree.levels[lvl_idx][node_idx].id << " ";
                    os << node_sigs.size() << std::endl;

                    IdType node_sigs_size = 0;
                    for (auto const& node_sigs_with_size : node_sigs) {
                        os << node_sigs_size << " ";
                        os << node_sigs_with_size.size() << std::endl;
                        for (auto const& sig : node_sigs_with_size) {
                            for (auto const& val : sig.first) {
                                os << val << " ";
                            }
                            os << sig.second << std::endl;
                        }
                        node_sigs_size += 1;
                    }
                    os << std::endl;
                }
            }
            return os;
        }

    template<typename IdType, typename EdgeWeightType>
        SignaturesForTreeBuilder<IdType, EdgeWeightType>& 
            SignaturesForTreeBuilder<IdType, EdgeWeightType>::with_part_cnt(IdType part_cnt) {

            this->part_cnt = part_cnt;
            return *this;
        }

    template<typename IdType, typename EdgeWeightType>
        SignaturesForTreeBuilder<IdType, EdgeWeightType>& 
            SignaturesForTreeBuilder<IdType, EdgeWeightType>::with_eps(RationalType eps) {

        this->eps = eps;
        return *this;
    }

    template<typename IdType, typename EdgeWeightType>
    SignaturesForTreeBuilder<IdType, EdgeWeightType>& SignaturesForTreeBuilder<IdType, EdgeWeightType>::with_signatures(
            std::vector<std::vector<SignatureMap>> const& signatures) {
        this->signatures = signatures;
        return *this;
    }

    template<typename IdType, typename EdgeWeightType>
        SignaturesForTree<IdType, EdgeWeightType> SignaturesForTreeBuilder<IdType, EdgeWeightType>::finish() {
            return SignaturesForTree<IdType, EdgeWeightType>(this->part_cnt, this->eps, this->tree, this->signatures);
        }

    template<typename IdType, typename EdgeWeightType>
        std::istream& operator>>(std::istream& is, SignaturesForTreeBuilder<IdType, EdgeWeightType>& builder) {
            using SizeType = IdType;

            SizeType part_cnt;
            int64_t eps_num;
            int64_t eps_denom;
            is >> part_cnt >> eps_num >> eps_denom;
            RationalType eps(eps_num, eps_denom);
            builder.with_part_cnt(part_cnt).with_eps(eps);
            auto signature_length = calculate_upper_component_size_bounds(
                    eps, builder.tree.tree_sizes[0][0], part_cnt).size();

            std::vector<std::vector<SignatureMap<IdType, EdgeWeightType>>> signatures;
            for (auto const& level : builder.tree.levels) {
                signatures.emplace_back(level.size());
            }

            for (SizeType node_idx = 0; node_idx < builder.tree.tree_sizes[0][0]; ++node_idx) {
                IdType node_id;
                SizeType size_cnt;
                is >> node_id >> size_cnt;
                auto node_idx_in_tree = builder.tree.get_node_idx(node_id);

                SizeType max_sig_size = 1;
                for (size_t sibling_idx = 0; sibling_idx <= node_idx_in_tree.second; ++sibling_idx) {
                    max_sig_size += builder.tree.tree_sizes[node_idx_in_tree.first][sibling_idx];
                }

                SignatureMap<IdType, EdgeWeightType>& node_sigs = signatures[node_idx_in_tree.first][node_idx_in_tree.second];
                node_sigs = SignatureMap<IdType, EdgeWeightType>(static_cast<size_t>(max_sig_size));

                for (SizeType size_idx = 0; size_idx < size_cnt; ++size_idx) {
                    SizeType size;
                    SizeType signature_cnt;
                    is >> size >> signature_cnt;

                    for (SizeType signature_idx = 0; signature_idx < signature_cnt; ++signature_idx) {
                        std::valarray<SizeType> signature(signature_length);
                        EdgeWeightType cut_cost;
                        for (auto& comp : signature) {
                            is >> comp;
                        }
                        is >> cut_cost;
                        node_sigs[static_cast<size_t>(size)][signature] = cut_cost;
                    }
                }
            }
            builder.with_signatures(signatures);

            return is;
        }
}
// Include template implementation.
//#include "Cut.ipp"
