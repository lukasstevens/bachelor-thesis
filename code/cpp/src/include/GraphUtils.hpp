/**
 * @file GraphUtils.hpp
 *
 * This file contains some utility functions for dealing with graph.
 * All theses utility functions are also in the namespace graph.
 */
#pragma once 

#include<algorithm>
#include<queue>
#include<random>
#include<tuple>
#include<vector>

#include "Graph.hpp"

namespace graph {

    /**
     * A union-find structure.
     */
    template<typename Id>
        struct UnionFind {
            private:
                std::vector<Id> parent; /**< Saves the parent of each node*/
                std::vector<Id> size; /**< Stores the size of the subtree of a node */

            public:
                /**
                 * Construct a union-find instance with \p node_count nodes.
                 * In the beginning all nodes are singletons.
                 * @param node_count The number of nodes.
                 */
                UnionFind(Id node_count) : parent(node_count), size(node_count, 1) {
                    for (Id node = 0; node < node_count; ++node) {
                        this->parent[node] = node;
                    }
                }

                /**
                 * Finds the representative of the set in which \p node lies.
                 * @param node The node.
                 * @returns The id of the representative.
                 */
                Id find(Id node) {
                    if(this->parent[node] == node) {
                        return node;
                    } else {
                        Id root = this->find(this->parent[node]);
                        this->parent[node] = root;
                        return root;
                    }
                }

                /**
                 * Union two sets.
                 * @param one_node Node in one set.
                 * @param other_node Node in the other set.
                 */
                void union_(Id one_node, Id other_node) {
                    Id one_root = this->find(one_node);
                    Id other_root = this->find(other_node);

                    if (one_root == other_root) {
                        return;
                    }
                    else if(this->size[one_root] <= this->size[other_root]) {
                        this->parent[one_root] = other_root;
                        this->size[other_root] += this->size[one_root];
                    } else {
                        this->union_(other_root, one_root);
                    }
                }

                /**
                 * Return all representatives.
                 * @returns The vector of representatives.
                 */
                std::vector<Id> roots() {
                    std::vector<Id> roots;
                    for (Id node = 0; node < parent.size(); ++node) {
                        if (this->parent[node] == node) {
                            roots.push_back(node);
                        }
                    }
                    return roots;
                }
        };

    /**
     * Do a heavy edge matching on the graph.
     * This visits all nodes in a random order and greedily matches each node with the adjacent node
     * to which the edge weight is the highest.
     * @param graph The graph to perform to search the matching in.
     * @param seed The seed to use to traverse the nodes randomly (default 0).
     * @returns The matching.
     */
    template<typename Id, typename NodeWeight, typename EdgeWeight, typename RandGen=std::mt19937_64>
        typename Graph<Id, NodeWeight, EdgeWeight>::Matching heavy_edge_matching(
                Graph<Id, NodeWeight, EdgeWeight> const& graph,
                size_t seed=0
                ) {

            std::vector<Id> visit_order(graph.node_cnt());
            for (Id node = 0; node < graph.node_cnt(); ++node) {
                visit_order.at(node) = node;
            }
            std::shuffle(visit_order.begin(), visit_order.end(), RandGen(seed));

            std::vector<bool> is_matched(graph.node_cnt());
            typename Graph<Id, NodeWeight, EdgeWeight>::Matching matching;

            // Rand gen for matching disconnected nodes.
            RandGen rand_gen(seed);
            std::uniform_int_distribution<Id> node_dist(0, graph.node_cnt() - 1);

            for (Id node : visit_order) {
                if (is_matched.at(node)) {
                    continue;
                }

                using Edge = std::pair<Id, EdgeWeight>;
                std::vector<Edge> inc_edges = graph.inc_edges(node);

                // Match disconnected nodes randomly.
                if (inc_edges.empty()) {
                    Id to_node = node_dist(rand_gen); 
                    if (node != to_node) {
                        inc_edges.emplace_back(to_node, 0);
                    }
                }

                auto comp = [](Edge one_edge, Edge other_edge){
                    return one_edge.second > other_edge.second;
                };
                std::sort(inc_edges.begin(), inc_edges.end(), comp);
                for (Edge edge : inc_edges) {
                    if (!is_matched.at(edge.first) && node != edge.first) {
                        matching.push_back(std::make_pair(node, edge.first));
                        is_matched[node] = true;
                        is_matched[edge.first] = true;
                        break;
                    }
                }
            }
            return matching;
        }

    /**
     * Contract \p graph to \p node_cnt nodes by iteratively contracting edges.
     * The edges to contract are found using heavy edge matching.
     * @param graph The graph to contract. This graph is NOT changed.
     * @param node_cnt The number of nodes of the contracted graph.
     * @param matching_seed The seed to use for the matching (default 0).
     * @returns The resulting contracted graph.
     * @see graph::heavy_edge_matching()
     */
    template<typename Id, typename NodeWeight, typename EdgeWeight,
        typename RandGen=std::mt19937_64>
            Graph<Id, NodeWeight, EdgeWeight> contract_to_n_nodes(
                    Graph<Id, NodeWeight, EdgeWeight> const& graph,
                    Id node_cnt, 
                    size_t matching_seed=0
                    ) {
                using Graph = Graph<Id, NodeWeight, EdgeWeight>;
                Graph contracted(graph);
                RandGen rand_gen(matching_seed);
                while(contracted.node_cnt() > node_cnt) {
                    auto matching =
                        heavy_edge_matching<Id, NodeWeight, EdgeWeight, RandGen>(contracted, matching_seed);
                    if (contracted.node_cnt() - matching.size() < node_cnt) {
                        matching = typename Graph::Matching(
                                matching.cbegin(),
                                matching.cbegin() + (contracted.node_cnt() - node_cnt)
                                );
                    }
                    contracted.contract_edges(matching);
                }
                return contracted;
            }

    /**
     * Find the MST of a graph using Kruskals algorithm.
     * @param graph The graph.
     * @returns The MST.
     */
    template<typename Id, typename NodeWeight, typename EdgeWeight>
        Graph<Id, NodeWeight, EdgeWeight> mst(
                Graph<Id, NodeWeight, EdgeWeight> const& graph) {
            Graph<Id, NodeWeight, EdgeWeight> mst_graph(graph.node_cnt());

            using Edge = std::tuple<EdgeWeight, Id, Id>;
            std::priority_queue<Edge,
                std::vector<Edge>, std::greater<Edge>> prio_q;
            for (auto const& edge : graph.edge_set()) {
                prio_q.emplace(std::get<2>(edge), std::get<0>(edge), std::get<1>(edge));
            }

            UnionFind<Id> union_find(graph.node_cnt());
            while(!prio_q.empty()) {
                EdgeWeight edge_weight;
                Id from_node;
                Id to_node;
                std::tie(edge_weight, from_node, to_node) = prio_q.top();
                prio_q.pop();
                
                if (union_find.find(from_node) != union_find.find(to_node)) {
                    mst_graph.edge_weight(from_node, to_node, edge_weight);
                    union_find.union_(from_node, to_node);
                }
            }

            std::vector<Id> roots = union_find.roots();
            for (Id root_idx = 1; root_idx < roots.size(); ++root_idx) {
                mst_graph.edge_weight(roots[0], roots[root_idx], 0);
            }

            return mst_graph;
        }

    /**
     * Calculates a Random Spanning Tree (RST) on \p graph.
     * @param graph The graph to use.
     * @param seed The seed for the random generator (default 0).
     * @returns The RST
     */
    template<typename Id, typename NodeWeight, typename EdgeWeight,
        typename RandGen=std::mt19937_64>
            Graph<Id, NodeWeight, EdgeWeight> rst(
                    Graph<Id, NodeWeight, EdgeWeight> const& graph,
                    size_t seed=0) {

                RandGen rand_gen(seed);

                Graph<Id, NodeWeight, EdgeWeight> rst_graph(graph.node_cnt());

                using Edge = std::tuple<size_t, Id, Id>;
                std::priority_queue<Edge,
                    std::vector<Edge>, std::greater<Edge>> prio_q;
                for (auto const& edge : graph.edge_set()) {
                    prio_q.emplace(rand_gen(), std::get<0>(edge), std::get<1>(edge));
                }

                UnionFind<Id> union_find(graph.node_cnt());
                while(!prio_q.empty()) {
                    size_t priority;
                    Id from_node;
                    Id to_node;
                    std::tie(priority, from_node, to_node) = prio_q.top();
                    prio_q.pop();

                    if (union_find.find(from_node) != union_find.find(to_node)) {
                        rst_graph.edge_weight(from_node, to_node,
                                graph.edge_weight(from_node, to_node));
                        union_find.union_(from_node, to_node);
                    }
                }

                std::vector<Id> roots = union_find.roots();
                for (Id root_idx = 1; root_idx < roots.size(); ++root_idx) {
                    rst_graph.edge_weight(roots[0], roots[root_idx], 0);
                }

                return rst_graph;
            }
}
