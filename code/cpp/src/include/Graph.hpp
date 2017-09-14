/** @file Graph.hpp */
#pragma once

#include<algorithm>
#include<cstdint>
#include<limits>
#include<list>
#include<set>
#include<stdexcept>
#include<string>
#include<unordered_map>
#include<vector>

#include<metis.h>
#include<kaHIP_interface.h>

#include "Cut.hpp"
#include "Partition.hpp"

/**
 * This namespace contains a structure representing an UNDIRECTED graph. This is used to wrap the 
 * \p Tree structure and simplify the interaction with the library.
 * All template types in this namespace are assumed to be integer.
 *
 * @see cut::Tree
 */
namespace graph {

    /**
     * A structure to store the result of a partitioning.
     * This stores the cost and the actual partitioning.
     * A partitioning is a vector with a length equal to the number of
     * nodes in the graph. The i-th entry of the vector indicates in which
     * partition the node with id i is.
     */
    template<typename Id=int32_t, typename EdgeWeight=int32_t>
        using PartitionResult = std::pair<EdgeWeight, std::vector<Id>>;

    /**
     * Typedef for the rational type.
     * @see cut::Rational
     */
    using Rational = cut::Rational;

    /**
     * This is a graph stored using the CSR format which is used by METIS and KaFFPa.
     * See the METIS manual in \p deps/metis/manual for a description of the format.
     */
    template<typename Idx>
        struct CsrGraph {
            public:
                Idx nvtxs;            
                std::vector<Idx> xadj;
                std::vector<Idx> adjncy;
                std::vector<Idx> vwgt;
                std::vector<Idx> adjwgt;

                /**
                 * Constructor.
                 * @param xadj The pointers into the adjncy array for each node.
                 * @param adjncy Describes the edges of the graph.
                 * @param vwgt Describes the node weights of the graph.
                 * @param adjwgt Describes the edge weights of the edges in \p adjncy.
                 */
                CsrGraph(
                        std::vector<Idx> xadj,
                        std::vector<Idx> adjncy,
                        std::vector<Idx> vwgt,
                        std::vector<Idx> adjwgt) :
                    nvtxs(static_cast<Idx>(vwgt.size())),
                    xadj(xadj),
                    adjncy(adjncy),
                    vwgt(vwgt),
                    adjwgt(adjwgt) {}
        };

    /**
     * CsrGraph for METIS.
     * @see CsrGraph
     */
    struct MetisCsrGraph : CsrGraph<idx_t> {
        private:

            /**
             * Partition the graph using the \p part_method METIS method.
             * @param part_methode The METIS method to use.
             * @param kparts The number of parts to partition into.
             * @param imbalance The maximum imbalance of the partitions.
             */
            PartitionResult<idx_t, idx_t> part_graph(decltype(METIS_PartGraphRecursive) part_method, idx_t kparts, real_t imbalance) {
                idx_t cut_cost;
                // Number of node constraints is always one.
                idx_t ncon = 1;
                std::vector<idx_t> partition(static_cast<size_t>(this->nvtxs));
                int res = part_method(
                        &(this->nvtxs), &ncon, &(this->xadj.at(0)), &(this->adjncy.at(0)), 
                        &(this->vwgt.at(0)), nullptr, &(this->adjwgt.at(0)), 
                        &kparts, nullptr, &imbalance, nullptr, 
                        &cut_cost, &partition.at(0)
                        );

                if (res != METIS_OK) {
                    throw std::runtime_error("Metis failed with error " + std::to_string(res));
                }
                return std::make_pair(cut_cost, partition);
            }

        public:
            /**
             * Cast constructor for ease of use
             * @param graph The CsrGraph to cast from.
             */
            MetisCsrGraph(CsrGraph<int> graph) : CsrGraph<int>(graph) {}

            /**
             * Constructor.
             * @param xadj The pointers into the adjncy array for each node.
             * @param adjncy Describes the edges of the graph.
             * @param vwgt Describes the node weights of the graph.
             * @param adjwgt Describes the edge weights of the edges in \p adjncy.
             */
            MetisCsrGraph(
                    std::vector<idx_t> xadj,
                    std::vector<idx_t> adjncy,
                    std::vector<idx_t> vwgt,
                    std::vector<idx_t> adjwgt
                    ) : CsrGraph<idx_t>(xadj, adjncy, vwgt, adjwgt) {}


            /**
             * Partition this graph with the given parameters using the METIS recursive method.
             * @param kparts The number of parts to partition into.
             * @param imbalance The maximum imbalance of the partition.
             * @returns The result of the partition.
             */
            PartitionResult<idx_t, idx_t> part_graph_recursive(idx_t kparts, real_t imbalance) {
                return part_graph(METIS_PartGraphRecursive, kparts, imbalance);
            }

            /**
             * Partition this graph with the given parameters using the METIS k-way method.
             * @param kparts The number of parts to partition into.
             * @param imbalance The maximum imbalance of the partition.
             * @returns The result of the partition.
             */
            PartitionResult<idx_t, idx_t> part_graph_kway(idx_t kparts, real_t imbalance) {
                return part_graph(METIS_PartGraphKway, kparts, imbalance);
            }
    };

    /**
     * CsrGraph for KaFFPa.
     * @see CsrGraph
     */
    struct KahipCsrGraph : CsrGraph<int> {
        public:
            /**
             * Cast constructor for ease of use.
             */
            KahipCsrGraph(CsrGraph<int> graph) : CsrGraph<int>(graph) {}

            /**
             * Constructor.
             * @param xadj The pointers into the adjncy array for each node.
             * @param adjncy Describes the edges of the graph.
             * @param vwgt Describes the node weights of the graph.
             * @param adjwgt Describes the edge weights of the edges in \p adjncy.
             */
            KahipCsrGraph(
                    std::vector<int> xadj,
                    std::vector<int> adjncy,
                    std::vector<int> vwgt,
                    std::vector<int> adjwgt
                    ) : CsrGraph<int>(xadj, adjncy, vwgt, adjwgt) {}

            /**
             * Partition this graph with the given parameters using KaFFPa. 
             * @param kparts The number of parts to partition into.
             * @param imbalance The maximum imbalance of the partition.
             * @param seed Set the seed for KaFFPa.
             * @returns The result of the partition.
             */
            PartitionResult<int, int> kaffpa(int kparts, double imbalance, int seed) {
                int cut_cost;
                std::vector<int> partition(static_cast<size_t>(this->nvtxs));
                ::kaffpa(
                        &(this->nvtxs), &(this->vwgt.at(0)), &(this->xadj.at(0)),
                        &(this->adjwgt.at(0)), &(this->adjncy.at(0)), &kparts,
                        &imbalance, true, seed, STRONG,
                        &cut_cost, &partition.at(0)
                        );
                return std::make_pair(cut_cost, partition);
            }
    };

    /**
     * Structure to store an undirected graph.
     * All template parameters are assumed to be integer.
     * Floats for EdgeWeight might work, but is not tested.
     */
    template<typename Id=int32_t, typename NodeWeight=int32_t, typename EdgeWeight=int32_t>
        struct Graph {
            public :

                /**
                 * A set of node ids.
                 */
                using NodeSet = std::set<Id>;

                /**
                 * A matching is a vector of edges.
                 */
                using Matching = std::vector<std::pair<Id, Id>>;

                /**
                 * Type for storing the result from a partitioning.
                 */
                using PartitionResult = PartitionResult<Id, EdgeWeight>;

                /**
                 * Stores an edge with its associated cost.
                 */
                using Edge = std::tuple<Id, Id, EdgeWeight>;

            private:

                std::vector<std::unordered_map<Id, EdgeWeight>> adjncy;
                std::vector<NodeSet> vrepr;
                std::vector<NodeWeight> vwgt;

                /**
                 * This constructor is only used internally.
                 * @param adjncy The graph.
                 * @param vwgt The node weights.
                 */
                Graph(
                        std::vector<std::unordered_map<Id, EdgeWeight>> adjncy,
                        std::vector<NodeWeight> vwgt
                     ) : adjncy(adjncy), vwgt(vwgt)
                {
                    for(Id i = 0; static_cast<size_t>(i) < adjncy.size(); ++i) {
                        vrepr.at(static_cast<size_t>(i)).insert(i);
                    }
                }

            public:

                /**
                 * Construct a graph with weighted nodes.
                 * @param vwgt The node weights.
                 */
                Graph(std::vector<NodeWeight> vwgt) : 
                    Graph(
                            std::vector<std::unordered_map<Id, EdgeWeight>>(),
                            vwgt
                         ) {}

                /**
                 * Construct a graph with \p node_cnt nodes.
                 * The node weights are set to one.
                 * @param node_cnt The number of nodes.
                 */
                Graph(Id node_cnt) : 
                    Graph() {
                        this->resize(node_cnt);
                    }

                /**
                 * Default constructor.
                 */
                Graph() = default;

                /**
                 * Getter for the node count of the graph.
                 * @returns The number of nodes.
                 */
                Id node_cnt() const {
                    return static_cast<Id>(vwgt.size());
                }

                /**
                 * Getter for the edge count of the graph.
                 * @returns The number of edges.
                 */
                Id edge_cnt() const {
                    return this->edge_set().size();
                }

                /**
                 * Getter for the node weights of the graph.
                 * @param node The node.
                 * @returns The node weight of \p node.
                 */
                NodeWeight node_weight(Id node) const {
                    return this->vwgt.at(node);
                }

                /**
                 * Setter for the node weights of the graph.
                 * @param node The node for which the weight has to be set.
                 * @param weight The new weight of the node.
                 */
                void node_weight(Id node, NodeWeight weight) {
                    this->vwgt.at(node) = weight;
                }

                /**
                 * Returns the set of nodes a certain node represents.
                 * This is only relevant if the graph was contracted, otherwhise each
                 * node just represents itself.
                 * @param node The node.
                 * @returns A set of node which \p node represents.
                 *
                 * @see contract_edges
                 */
                NodeSet node_repr(Id node) const {
                    return this->vrepr.at(node);
                }

                /**
                 * Checks if the given edge exists in the graph.
                 * Keep in mind that the graph is undirected.
                 * @param from_node The first node.
                 * @param to_node The second node.
                 * @returns A boolean indicating whether the edge exists.
                 */
                bool exists_edge(Id from_node, Id to_node) const {
                    return this->adjncy.at(from_node).find(to_node) !=
                        this->adjncy.at(from_node).cend();
                }

                /**
                 * A getter for the weight of an edge.
                 * @param from_node One node of the edge.
                 * @param to_node Other node of the edge.
                 * @returns The weight of the edge.
                 */
                EdgeWeight edge_weight(Id from_node, Id to_node) const {
                    return this->adjncy.at(from_node).at(to_node);
                }

                /**
                 * A setter for the weight of an edge.
                 * @param from_node One node of the edge.
                 * @param to_node Other node of the edge.
                 * @param weight The weight of the edge.
                 */
                void edge_weight(Id from_node, Id to_node, EdgeWeight weight) {
                    this->adjncy.at(from_node)[to_node] = weight;
                    this->adjncy.at(to_node)[from_node] = weight;
                }

                /**
                 * An additive setter for the weight of an edge.
                 * Constructs the edge with initial weight 0 if necessary.
                 * @param from_node One node of the edge.
                 * @param to_node Other node of the edge.
                 * @param weight The weight of the edge.
                 */
                void add_edge_weight(Id from_node, Id to_node, EdgeWeight weight) {
                    this->adjncy.at(from_node)[to_node] += weight;
                    this->adjncy.at(to_node)[from_node] += weight;
                }

                /**
                 * Erase an edge of the graph.
                 * Keep in mind that this erases both directions, since the graph
                 * is undirected.
                 * @param from_node The first node of the edge.
                 * @param to_node The other node of the edge.
                 */
                void remove_edge(Id from_node, Id to_node) {
                    this->adjncy.at(from_node).erase(to_node);
                    this->adjncy.at(to_node).erase(from_node);
                }

                /**
                 * Getter for the edge set of the graph.
                 * This operation is expensive since it iterates through all nodes and
                 * their edges.
                 * @returns A vector representing all edges in the graph.
                 */
                std::vector<Edge> edge_set() const {
                    std::vector<Edge> edge_set;
                    for (Id node = 0; node < this->node_cnt(); ++node) {
                        for (auto const& inc_edge : this->inc_edges(node)) {
                            if (inc_edge.first >= node) {
                                edge_set.emplace_back(node, inc_edge.first, inc_edge.second);
                            }
                        }
                    }
                    return edge_set;
                }

                /**
                 * Getter for the adjacent nodes of a node.
                 * @param node The node.
                 * @returns A vector of ids identifying the adjacent nodes in the graph
                 */
                std::vector<Id> adj_nodes(Id node) const {
                    std::vector<Id> adj_nodes;
                    adj_nodes.reserve(this->adjncy.at(node).size());
                    for (auto const& edge : this->adjncy.at(node)) {
                        adj_nodes.push_back(edge.first);
                    }
                    return adj_nodes;
                }

                /**
                 * Getter for the incident edges of a node.
                 * @param node The node.
                 * @returns A vector of target nodes with the associated weights.
                 */
                std::vector<std::pair<Id, EdgeWeight>> inc_edges(Id node) const {
                    return std::vector<std::pair<Id, EdgeWeight>>(
                            this->adjncy.at(node).cbegin(), this->adjncy.at(node).cend());
                }

                /**
                 * Resize the graph to \p node_cnt nodes.
                 * This also deletes edges as necessary.
                 * @param node_cnt The number of nodes to resize to.
                 */
                void resize(Id node_cnt) {
                    auto n = static_cast<size_t>(node_cnt);
                    this->vrepr.resize(n);
                    for (Id i = 0; i < node_cnt; ++i) {
                        if (vrepr.at(static_cast<size_t>(i)).empty()) {
                            vrepr.at(static_cast<size_t>(i)).insert(i);
                        }
                    }
                    this->vwgt.resize(n, 1);

                    std::vector<std::unordered_map<Id, EdgeWeight>> new_adjncy(n);
                    for (Id i = 0; i < node_cnt; ++i) {
                        auto const& i_adjncy = new_adjncy.at(i);
                        for (auto const& edge : i_adjncy) {
                            if (edge.first < n) {
                                new_adjncy.at(i)[edge.first] = edge.second;
                                new_adjncy.at(edge.first)[i] = edge.second;
                            }
                        }
                    }
                    this->adjncy = new_adjncy;
                }

                /**
                 * Convert the graph in this format to a graph in CSR format.
                 */
                template<typename Idx> 
                    CsrGraph<Idx> to_foreign_graph() const {
                        Id const node_cnt = this->node_cnt();
                        Id const edge_cnt = this->edge_cnt();

                        std::vector<Idx> metis_vwgt(this->vwgt.cbegin(), this->vwgt.cend());
                        std::vector<Idx> metis_xadj(static_cast<size_t>(node_cnt + 1));
                        std::vector<Idx> metis_adjncy;
                        metis_adjncy.reserve(static_cast<size_t>(edge_cnt));
                        std::vector<Idx> metis_adjwgt;
                        metis_adjwgt.reserve(static_cast<size_t>(edge_cnt));

                        for (Id curr_node = 0; curr_node < node_cnt; ++curr_node) {
                            size_t const curr_node_st = static_cast<size_t>(curr_node);
                            metis_xadj.at(curr_node_st + 1) = metis_xadj.at(curr_node_st);
                            for (auto const& inc_edge : this->adjncy.at(curr_node)) {
                                metis_adjncy.push_back(inc_edge.first); 
                                metis_adjwgt.push_back(inc_edge.second);
                                metis_xadj.at(curr_node_st + 1) += 1;
                            }
                        }
                        return CsrGraph<Idx>(metis_xadj, metis_adjncy, metis_vwgt, metis_adjwgt);
                    }

                /**
                 * Convert a partition on the current graph into the real partition.
                 * This takes previous contractions into account.
                 * Keep in mind that this function does not warn you if the partitioning has
                 * to many nodes.
                 * @param partitioning The partitioning to convert.
                 * @returns The converted partitioning.
                 */
                std::vector<Id> convert_part_to_node_repr(std::vector<Id> const& partitioning) const {
                    size_t node_repr_count = 0;
                    for (Id node = 0; node < this->node_cnt(); ++node) {
                        node_repr_count += this->node_repr(node).size(); 
                    }

                    std::vector<Id> part_repr(node_repr_count);
                    for (Id node = 0; node < this->node_cnt(); ++node) {
                        for (auto const& node_rep : this->node_repr(node)) {
                            part_repr.at(node_rep) = partitioning.at(node);
                        }
                    }
                    return part_repr;
                }

                /**
                 * Convert this graph to a METIS CsrGraph.
                 */
                MetisCsrGraph to_metis_graph() const {
                    return static_cast<MetisCsrGraph>(this->to_foreign_graph<idx_t>());
                }

                /**
                 * Partition this graph using the METIS recursive method.
                 * Pay attention that this will convert the graph to a CSR graph which only uses
                 * 32-bit integers to represent the graph.
                 * @param kparts The number of parts to partition into.
                 * @param imbalance The desired maximum imbalance of the partitions.
                 * @returns The partition.
                 */ 
                PartitionResult partition_metis_recursive(idx_t kparts, Rational imbalance) const {
                    auto const part_res = this->to_metis_graph().part_graph_recursive(
                            kparts, static_cast<real_t>(1 + imbalance.get_d()));
                    return part_res;
                } 

                /**
                 * Partition this graph using the METIS k-way method.
                 * Pay attention that this will convert the graph to a CSR graph which only uses
                 * 32-bit integers to represent the graph.
                 * @param kparts The number of parts to partition into.
                 * @param imbalance The desired maximum imbalance of the partitions.
                 * @returns The partition.
                 */ 
                PartitionResult partition_metis_kway(idx_t kparts, Rational imbalance) const {
                    auto const part_res = this->to_metis_graph().part_graph_kway(
                            kparts, static_cast<real_t>(1 + imbalance.get_d()));
                    return part_res;
                } 

                /**
                 * Convert this graph to a Kahip CsrGraph.
                 */
                KahipCsrGraph to_kahip_graph() const {
                    return static_cast<KahipCsrGraph>(this->to_foreign_graph<int>());
                }

                /**
                 * Partition this graph using KaFFPa.
                 * Pay attention that this will convert the graph to a CSR graph which only uses
                 * 32-bit integers to represent the graph.
                 * @param kparts The number of parts to partition into.
                 * @param imbalance The desired maximum imbalance of the partitions.
                 * @param seed An optional seed for KaFFPa (default 0).
                 * @returns The partition.
                 */ 
                PartitionResult partition_kaffpa(int kparts, Rational imbalance, long seed=0) const {
                    auto const part_res = this->to_kahip_graph().kaffpa(kparts, imbalance.get_d(), seed);
                    return part_res;
                } 

                /**
                 * Checks if the graph represents a tree.
                 * @returns A boolean indicating whether the graph is a tree.
                 */
                bool is_tree() const {
                    std::list<std::pair<Id, Id>> queue;
                    queue.emplace_back(std::make_pair(0, 0));
                    std::vector<bool> visited(this->node_cnt());
                    while(!queue.empty()) {
                        Id curr_node;
                        Id previous_node;
                        std::tie(curr_node, previous_node) = queue.front();
                        queue.pop_front();
                        if (visited.at(curr_node)) {
                            return false;
                        }
                        visited.at(curr_node) = true;
                        for (auto const& neighbor_node : this->adj_nodes(curr_node)) {
                            if (neighbor_node != previous_node) {
                                queue.emplace_back(std::make_pair(neighbor_node, curr_node));
                            }
                        }
                    }
                    return true;
                }

                /**
                 * Converts the graph to a tree in the \p Tree format.
                 * @param root The desired root (default 0).
                 * @returns The tree.
                 * @see cut::Tree
                 */
                cut::Tree<Id, NodeWeight, EdgeWeight> to_tree(Id root=0) const {
                    if (!this->is_tree()) {
                        throw std::logic_error("The graph is not a tree.");
                    }

                    std::map<Id, std::map<Id, EdgeWeight>> tree_map;
                    std::map<Id, NodeWeight> node_weight;
                    for (Id node = 0; node < this->node_cnt(); ++node) {
                        auto const inc_edges = this->inc_edges(node);
                        tree_map[node].insert(inc_edges.cbegin(), inc_edges.cend());
                        node_weight[node] = this->node_weight(node);
                    }
                    return cut::Tree<Id, NodeWeight, EdgeWeight>::build_tree(tree_map, node_weight, root);
                }


                /**
                 * Partition the graph using the method of Feldmann and Foschini.
                 * This only works if the graph is a tree.
                 * @param kparts The number of parts to partition into.
                 * @param imbalance The desired maximum imbalance of the partitioning.
                 * @param root The desired root (default 0).
                 * @returns The partitioning.
                 * 
                 * @throws std::logic_error if the graph is not a tree.
                 * @see is_tree()
                 */
                PartitionResult partition(Id kparts, Rational imbalance, Id root=0) const {
                    cut::Tree<Id, NodeWeight, EdgeWeight> tree = this->to_tree(root);
                    auto signatures = tree.cut(imbalance, kparts, true);

                    std::vector<std::set<Id>> partitioning;
                    typename cut::Tree<Id, NodeWeight, EdgeWeight>::Signature signature;
                    EdgeWeight cut_cost;
                    std::tie(partitioning, signature, cut_cost) = part::calculate_best_packing(signatures);
                    std::vector<Id> partitioning_formatted(this->node_cnt());
                    for (Id part_idx = 0; part_idx < partitioning.size(); ++part_idx) {
                        for (auto const& node : partitioning[part_idx]) {
                            partitioning_formatted.at(node) = part_idx;
                        }
                    }
                    return std::make_pair(cut_cost, partitioning_formatted);
                }

                /**
                 * Calculate the cost of a partition in the graph.
                 * Useful if the partitioning was calculated on a spanning tree of the graph
                 * and the cost is therefore not accurate.
                 * @param partitioning The partitioning.
                 * @returns The accumalated weight of the edges cut.
                 */
                EdgeWeight partition_cost(std::vector<Id> const& partitioning) const {
                    EdgeWeight cost = 0;
                    for (Id node = 0; node < this->node_cnt(); ++node) {
                        for (auto const& edge : this->inc_edges(node)) {
                            if (edge.first >= node &&
                                    partitioning.at(node) != partitioning.at(edge.first)) {
                                cost += edge.second;
                            }
                        }
                    }
                    return cost;
                }

                /**
                 * Contracts the edges in \p matching.
                 * This will change which nodes a node represents and
                 * also the id of every node.
                 * @param matching The matching.
                 */
                void contract_edges(Matching const& matching) {
                    std::vector<bool> is_matched(this->node_cnt());
                    for (auto const& edge : matching) {
                        auto match = [&is_matched](Id node){
                            if (!is_matched.at(node)) {
                                is_matched[node] = true;
                            } else {
                                throw std::logic_error("Not a matching.");
                            }
                        };
                        match(edge.first);
                        match(edge.second);
                    }

                    Graph<Id, NodeWeight, EdgeWeight> result_graph(this->node_cnt() - matching.size());
                    std::vector<bool> is_in_result_graph(this->node_cnt());
                    std::vector<Id> node_in_result_graph(this->node_cnt());

                    Id curr_node = 0;
                    for (auto const& edge : matching) {
                        result_graph.node_weight(curr_node, 
                                this->node_weight(edge.first) + this->node_weight(edge.second));

                        NodeSet new_vrepr;
                        std::set_union(this->vrepr.at(edge.first).cbegin(), this->vrepr.at(edge.first).cend(),
                                this->vrepr.at(edge.second).cbegin(), this->vrepr.at(edge.second).cend(),
                                std::inserter(new_vrepr, new_vrepr.end()));
                        result_graph.vrepr.at(curr_node) = new_vrepr;

                        is_in_result_graph.at(edge.first) = true;
                        is_in_result_graph.at(edge.second) = true;
                        node_in_result_graph.at(edge.first) = curr_node;
                        node_in_result_graph.at(edge.second) = curr_node;
                        curr_node += 1;
                    }

                    for (Id node = 0; node < this->node_cnt(); ++node) {
                        if (!is_in_result_graph[node]) {
                            result_graph.node_weight(curr_node, this->node_weight(node));
                            result_graph.vrepr.at(curr_node) = this->node_repr(node);
                            is_in_result_graph[node] = true;
                            node_in_result_graph.at(node) = curr_node;
                            curr_node += 1;
                        }
                    }

                    for (Id node = 0; node < this->node_cnt(); ++node) {
                        for (auto const& inc_edge : this->inc_edges(node)) {
                            Id const from_in_res = node_in_result_graph.at(node);
                            Id const to_in_res = node_in_result_graph.at(inc_edge.first);
                            if (inc_edge.first >= node && from_in_res != to_in_res) {
                                result_graph.add_edge_weight(
                                        from_in_res, to_in_res, inc_edge.second);
                            }
                        }
                    }
                    *this = result_graph;
                }
        };
}

