#pragma once 

#include<algorithm>
#include<queue>
#include<random>
#include<tuple>
#include<vector>

#include "Graph.hpp"

namespace graph {

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
            for (Id node : visit_order) {
                using Edge = std::pair<Id, EdgeWeight>;
                auto comp = [](Edge one_edge, Edge other_edge){
                    return one_edge.second > other_edge.second;
                };
                std::vector<Edge> inc_edges = graph.inc_edges(node);
                std::sort(inc_edges.begin(), inc_edges.end(), comp);
                for (Edge edge : inc_edges) {
                    if (!is_matched.at(node) && !is_matched.at(edge.first)) {
                        matching.push_back(std::make_pair(node, edge.first));
                        is_matched[node] = true;
                        is_matched[edge.first] = true;
                    }
                }
            }
            return matching;
        }

    template<typename Id, typename NodeWeight, typename EdgeWeight,
        typename RandGen=std::mt19937_64>
        Graph<Id, NodeWeight, EdgeWeight> contract_to_n_nodes(
                Graph<Id, NodeWeight, EdgeWeight> const& graph,
                Id node_cnt, 
                size_t matching_seed=0
                ) {
            using Graph = Graph<Id, NodeWeight, EdgeWeight>;
            Graph contracted(graph);
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

    template<typename Id, typename NodeWeight, typename EdgeWeight>
        Graph<Id, NodeWeight, EdgeWeight> mst(
                Graph<Id, NodeWeight, EdgeWeight> const& graph) {
            Graph<Id, NodeWeight, EdgeWeight> mst_graph(graph.node_cnt());

            using Edge = std::tuple<EdgeWeight, Id, Id>;
            std::priority_queue<Edge,
                std::vector<Edge>, std::greater<Edge>> prio_q;
            prio_q.emplace(0, 0, 0);

            std::vector<bool> visited(graph.node_cnt());
            Id visited_cnt = 0;
            while(visited_cnt < graph.node_cnt()) {
                EdgeWeight edge_weight;
                Id from;
                Id to;
                std::tie(edge_weight, from, to) = prio_q.top();
                prio_q.pop();
                if (!visited.at(to)) {
                    visited_cnt += 1;
                    mst_graph.edge_weight(from, to, edge_weight);
                    visited.at(to) = true;
                }
                for (auto const& neighbor :
                        graph.inc_edges(to)) {
                    if (!visited.at(neighbor.first)) {
                        prio_q.emplace(neighbor.second, to, neighbor.first);
                    }
                }
            }
            mst_graph.remove_edge(0, 0);
            return mst_graph;
        }

    template<typename Id, typename NodeWeight, typename EdgeWeight,
        typename RandGen=std::mt19937_64>
        Graph<Id, NodeWeight, EdgeWeight> rst(
                Graph<Id, NodeWeight, EdgeWeight> const& graph,
                size_t seed=0) {

            RandGen rand_gen(seed);

            Graph<Id, NodeWeight, EdgeWeight> rst_graph(graph.node_cnt());

            using EdgeWithPrio = std::tuple<EdgeWeight, EdgeWeight, Id, Id>;
            std::priority_queue<EdgeWithPrio, std::vector<EdgeWithPrio>,
                std::greater<EdgeWithPrio>> prio_q;
            prio_q.emplace(0, 0, 0, 0);

            std::vector<bool> visited(graph.node_cnt());
            Id visited_cnt = 0;
            while(visited_cnt < graph.node_cnt()) {
                EdgeWeight priority;
                EdgeWeight edge_weight;
                Id from;
                Id to;
                std::tie(priority, edge_weight, from, to) = prio_q.top();
                prio_q.pop();
                if (!visited.at(to)) {
                    visited_cnt += 1;
                    rst_graph.edge_weight(from, to, edge_weight);
                    visited.at(to) = true;
                }
                for (auto const& neighbor :
                        graph.inc_edges(to)) {
                    if (!visited.at(neighbor.first)) {
                        prio_q.emplace(rand_gen(), neighbor.second, to, neighbor.first);
                    }
                }
            }
            rst_graph.remove_edge(0, 0);
            return rst_graph;
        }
}
