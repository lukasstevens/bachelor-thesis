/** @file Graph.hpp */
#pragma once

#include<algorithm>
#include<limits>
#include<set>
#include<stdexcept>
#include<string>
#include<unordered_map>
#include<vector>

#include<metis.h>
#include<kaHIP_interface.h>

namespace graph {

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
        public:
            MetisCsrGraph(CsrGraph<int> graph) : CsrGraph<int>(graph) {}

            MetisCsrGraph(
                    std::vector<idx_t> xadj,
                    std::vector<idx_t> adjncy,
                    std::vector<idx_t> vwgt,
                    std::vector<idx_t> adjwgt
                    ) : CsrGraph<idx_t>(xadj, adjncy, vwgt, adjwgt) {}

            void part_graph(decltype(METIS_PartGraphRecursive) part_method, idx_t kparts, real_t imbalance) {
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
            }

            void part_graph_recursive(idx_t kparts, real_t imbalance) {
                part_graph(METIS_PartGraphRecursive, kparts, imbalance);
            }

            void part_graph_kway(idx_t kparts, real_t imbalance) {
                part_graph(METIS_PartGraphKway, kparts, imbalance);
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

            void kaffpa(int kparts, double imbalance, int seed) {
                int cut_cost;
                std::vector<int> partition(static_cast<size_t>(this->nvtxs));
                ::kaffpa(
                        &(this->nvtxs), &(this->vwgt.at(0)), &(this->xadj.at(0)),
                        &(this->adjwgt.at(0)), &(this->adjncy.at(0)), &kparts,
                        &imbalance, true, seed, STRONG,
                        &cut_cost, &partition.at(0)
                        );
            }
    };

    template<typename Id=int, typename NodeWeight=int, typename EdgeWeight=int>
        struct Graph {
            public :
                using NodeSet = std::set<Id>;
                using Matching = std::vector<std::pair<Id, Id>>;

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
                        edge_cnt += adj_nodes.second.size();
                    }
                    // Edges are counted at both endpoints.
                    return edge_cnt / 2;
                }

                void add_node(NodeWeight node_weight) {
                    auto node_id = this->node_cnt();
                    vwgt.push_back(node_weight);
                    vrepr.emplace_back({node_id});
                }

                void add_node() {
                    add_node(1);
                }

                NodeWeight node_weight(Id node) const {
                    return this->vwgt.at(node);
                }

                void node_weight(Id node, NodeWeight weight) {
                    this->vwgt.at(node) = weight;
                }


                NodeSet node_repr(Id node) {
                    return this->vrepr.at(node);
                }

                bool exists_edge(Id from_node, Id to_node) const {
                    return this->adjncy.at(from_node).find(to_node) 
                        != this->adjncy.at(from_node).cend();
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

                MetisCsrGraph to_metis_graph() const {
                    return static_cast<MetisCsrGraph>(this->to_foreign_graph<idx_t>());
                }

                KahipCsrGraph to_kahip_graph() const {
                    return static_cast<KahipCsrGraph>(this->to_foreign_graph<int>());
                }

                Graph<Id, NodeWeight, EdgeWeight> contract_edges(Matching const& matching) {
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

                    return result_graph;
                }
        };
}

