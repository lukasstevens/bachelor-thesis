#pragma once 

#include<algorithm>
#include<queue>
#include<random>
#include<tuple>
#include<vector>

#include "Graph.hpp"

namespace graph {

    template<typename Id, typename NodeWeight, typename EdgeWeight>
        typename Graph<Id, NodeWeight, EdgeWeight>::Matching heavy_edge_matching(
                Graph<Id, NodeWeight, EdgeWeight> const& graph,
                size_t matching_seed=0
                ) {

            using NodeWithDegree = std::pair<size_t, Id>;
            std::priority_queue<NodeWithDegree, 
                std::vector<NodeWithDegree>, std::greater<NodeWithDegree>> prio_q;
            
            for (Id node = 0; node < graph.node_cnt(); ++node) {
                prio_q.push(std::make_pair(graph.adj_nodes(node).size(), node));
            }

            std::vector<bool> is_matched(graph.node_cnt());
            typename Graph<Id, NodeWeight, EdgeWeight>::Matching matching;
            while(!prio_q.empty()) {
                size_t curr_deg;
                Id curr_node;
                std::tie(curr_deg, curr_node) = prio_q.top();
                prio_q.pop();

                using Edge = std::pair<Id, EdgeWeight>;
                auto comp = [](Edge one_edge, Edge other_edge){
                    return one_edge.second > other_edge.second;
                };
                std::vector<Edge> inc_edges = graph.inc_edges(curr_node);
                std::sort(inc_edges.begin(), inc_edges.end(), comp);
                for (Edge edge : inc_edges) {
                    if (!is_matched.at(curr_node) && !is_matched.at(edge.first)) {
                        matching.push_back(std::make_pair(curr_node, edge.first));
                        is_matched[curr_node] = true;
                        is_matched[edge.first] = true;
                    }
                }
            }
            return matching;
        }

    template<typename Id, typename NodeWeight, typename EdgeWeight>
        Graph<Id, NodeWeight, EdgeWeight> contract_to_n_nodes(
                Graph<Id, NodeWeight, EdgeWeight> const& graph,
                Id node_cnt, 
                size_t matching_seed=0
                ) {
            Graph<Id, NodeWeight, EdgeWeight> graph_cp(graph);
            while(graph_cp.node_cnt() > node_cnt) {
                auto matching = heavy_edge_matching(graph_cp);
                if (graph.node_cnt() - matching.size() < node_cnt) {
                    matching = Matching(matching.cbegin(), matching.cbegin() + (graph.node_cnt() - node_cnt));
                }
                graph_cp.contract_edges(matching);
            }
            return graph_cp;
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
                    if (!visited.at(neighbor)) {
                        prio_q.emplace(rand_gen(), neighbor.second, to, neighbor.first);
                    }
                }
            }
            rst_graph.remove_edge(0, 0);
            return rst_graph;
        }
}
