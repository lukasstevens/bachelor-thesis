#include<algorithm>
#include<queue>
#include<vector>

#include "Graph.hpp"

namespace graph {

    template<typename Id, typename NodeWeight, typename EdgeWeight>
        typename Graph<Id, NodeWeight, EdgeWeight>::Matching heavy_edge_matching(
                Graph<Id, NodeWeight, EdgeWeight> graph) {

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
}
