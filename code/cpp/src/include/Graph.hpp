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

namespace graph {

    template<typename Id=int32_t, typename EdgeWeight=int32_t>
        using PartitionResult = std::pair<EdgeWeight, std::vector<Id>>;

    using Rational = cut::Rational;

    template<typename Idx>
        struct CsrGraph {
            public:
                Idx nvtxs;            
                std::vector<Idx> xadj;
                std::vector<Idx> adjncy;
                std::vector<Idx> vwgt;
                std::vector<Idx> adjwgt;

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

    struct MetisCsrGraph : CsrGraph<idx_t> {
        private:
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
            MetisCsrGraph(CsrGraph<int> graph) : CsrGraph<int>(graph) {}

            MetisCsrGraph(
                    std::vector<idx_t> xadj,
                    std::vector<idx_t> adjncy,
                    std::vector<idx_t> vwgt,
                    std::vector<idx_t> adjwgt
                    ) : CsrGraph<idx_t>(xadj, adjncy, vwgt, adjwgt) {}


            PartitionResult<idx_t, idx_t> part_graph_recursive(idx_t kparts, real_t imbalance) {
                return part_graph(METIS_PartGraphRecursive, kparts, imbalance);
            }

            PartitionResult<idx_t, idx_t> part_graph_kway(idx_t kparts, real_t imbalance) {
                return part_graph(METIS_PartGraphKway, kparts, imbalance);
            }
    };

    struct KahipCsrGraph : CsrGraph<int> {
        public:
            KahipCsrGraph(CsrGraph<int> graph) : CsrGraph<int>(graph) {}

            KahipCsrGraph(
                    std::vector<int> xadj,
                    std::vector<int> adjncy,
                    std::vector<int> vwgt,
                    std::vector<int> adjwgt
                    ) : CsrGraph<int>(xadj, adjncy, vwgt, adjwgt) {}

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

    template<typename Id=int32_t, typename NodeWeight=int32_t, typename EdgeWeight=int32_t>
        struct Graph {
            public :
                using NodeSet = std::set<Id>;
                using Matching = std::vector<std::pair<Id, Id>>;
                using PartitionResult = PartitionResult<Id, EdgeWeight>;

            private:

                std::vector<std::unordered_map<Id, EdgeWeight>> adjncy;
                std::vector<NodeSet> vrepr;
                std::vector<NodeWeight> vwgt;

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

                Graph(std::vector<NodeWeight> vwgt) : 
                    Graph(
                            std::vector<std::unordered_map<Id, EdgeWeight>>(),
                            vwgt
                         ) {}

                Graph(Id node_cnt) : 
                    Graph() {
                        this->resize(node_cnt);
                    }

                Graph() = default;

                Id node_cnt() const {
                    return static_cast<Id>(vwgt.size());
                }

                Id edge_cnt() const {
                    Id edge_cnt = 0;
                    for (auto const& adj_nodes : adjncy) {
                        edge_cnt += adj_nodes.size();
                    }
                    // Edges are counted at both endpoints.
                    return edge_cnt / 2;
                }

                NodeWeight node_weight(Id node) const {
                    return this->vwgt.at(node);
                }

                void node_weight(Id node, NodeWeight weight) {
                    this->vwgt.at(node) = weight;
                }

                NodeSet node_repr(Id node) const {
                    return this->vrepr.at(node);
                }

                bool exists_edge(Id from_node, Id to_node) const {
                    return this->adjncy.at(from_node).find(to_node) !=
                        this->adjncy.at(from_node).cend();
                }

                EdgeWeight edge_weight(Id from_node, Id to_node) const {
                    return this->adjncy.at(from_node).at(to_node);
                }


                void edge_weight(Id from_node, Id to_node, EdgeWeight weight) {
                    this->adjncy.at(from_node)[to_node] = weight;
                    this->adjncy.at(to_node)[from_node] = weight;
                }

                void add_edge_weight(Id from_node, Id to_node, EdgeWeight weight) {
                    this->adjncy.at(from_node)[to_node] += weight;
                    this->adjncy.at(to_node)[from_node] += weight;
                }

                void remove_edge(Id from_node, Id to_node) {
                    this->adjncy.at(from_node).erase(to_node);
                    this->adjncy.at(to_node).erase(from_node);
                }

                std::vector<Id> adj_nodes(Id node) const {
                    std::vector<Id> adj_nodes;
                    adj_nodes.reserve(this->adjncy.at(node).size());
                    for (auto const& edge : this->adjncy.at(node)) {
                        adj_nodes.push_back(edge.first);
                    }
                    return adj_nodes;
                }

                std::vector<std::pair<Id, EdgeWeight>> inc_edges(Id node) const {
                    return std::vector<std::pair<Id, EdgeWeight>>(
                            this->adjncy.at(node).cbegin(), this->adjncy.at(node).cend());
                }

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

                PartitionResult convert_part_to_node_repr(PartitionResult const& part_result) const {
                    size_t node_repr_count = 0;
                    for (Id node = 0; node < this->node_cnt(); ++node) {
                        node_repr_count += this->node_repr(node).size(); 
                    }

                    std::vector<Id> part_repr(node_repr_count);
                    for (Id node = 0; node < this->node_cnt(); ++node) {
                        for (auto const& node_rep : this->node_repr(node)) {
                            part_repr.at(node_rep) = part_result.second.at(node);
                        }
                    }
                    return std::make_pair(part_result.first, part_repr);
                }

                MetisCsrGraph to_metis_graph() const {
                    return static_cast<MetisCsrGraph>(this->to_foreign_graph<idx_t>());
                }

                PartitionResult partition_metis_recursive(idx_t kparts, Rational imbalance) const {
                    auto const part_res = this->to_metis_graph().part_graph_recursive(
                            kparts, static_cast<real_t>(1 + imbalance.get_d()));
                    return this->convert_part_to_node_repr(part_res);
                } 

                PartitionResult partition_metis_kway(idx_t kparts, Rational imbalance) const {
                    auto const part_res = this->to_metis_graph().part_graph_kway(
                            kparts, static_cast<real_t>(1 + imbalance.get_d()));
                    return this->convert_part_to_node_repr(part_res);
                } 

                KahipCsrGraph to_kahip_graph() const {
                    return static_cast<KahipCsrGraph>(this->to_foreign_graph<int>());
                }

                PartitionResult partition_kaffpa(int kparts, Rational imbalance, long seed=0) const {
                    auto const part_res = this->to_kahip_graph().kaffpa(kparts, imbalance.get_d(), seed);
                    return this->convert_part_to_node_repr(part_res);
                } 

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


                PartitionResult partition(Id kparts, Rational imbalance, Id root=0) const {
                    cut::Tree<Id, NodeWeight, EdgeWeight> tree = this->to_tree(root);
                    auto signatures = tree.cut(imbalance, kparts);

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
                    return this->convert_part_to_node_repr(
                            std::make_pair(cut_cost, partitioning_formatted));
                }

                EdgeWeight partition_cost(std::vector<Id> const& partition) {
                    EdgeWeight cost = 0;
                    for (Id node = 0; node < this->node_cnt(); ++node) {
                        for (auto const& edge : this->inc_edges(node)) {
                            if (edge.first >= node &&
                                    partition.at(node) != partition.at(edge.first)) {
                                cost += edge.second;
                            }
                        }
                    }
                    return cost;
                }

                void contract_edges(Matching const& matching) {
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

                    for (Id node = 0; static_cast<size_t>(node) < is_in_result_graph.size(); ++node) {
                        if (!is_in_result_graph[node]) {
                            result_graph.node_weight(curr_node, this->node_weight(node));
                            result_graph.vrepr.at(curr_node) = this->node_repr(node);
                            is_in_result_graph[node] = true;
                            node_in_result_graph.at(node) = curr_node;
                            curr_node += 1;
                        }
                    }

                    for (Id node = 0; static_cast<size_t>(node) < this->node_cnt(); ++node) {
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

