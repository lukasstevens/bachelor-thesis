#include<random>
#include<limits.h>

#include<Graph.hpp>

namespace graphgen {
    using RandGen = std::mt19937_64;

    template<typename T>
        T gen_rand_in_range(
                RandGen& rand_gen, std::pair<T, T> range) {
            std::uniform_int_distribution<T> dist(range.first, range.second);    
            return dist(rand_gen);
        }

    template<typename Id, typename NodeWeight, typename EdgeWeight>
        graph::Graph<Id, NodeWeight, EdgeWeight> tree_rand_attach(
                Id node_cnt,
                std::pair<NodeWeight, NodeWeight> node_weight_range = {1, 2},
                std::pair<EdgeWeight, EdgeWeight> edge_weight_range = {1, 101},
                Id max_degree = std::numeric_limits<Id>::max(),
                long seed = 0
                ) {
            RandGen rand_gen(seed);

            graph::Graph<Id, NodeWeight, EdgeWeight> graph;
            graph.resize(node_cnt);
            graph.node_weight(0, 
                    gen_rand_in_range<NodeWeight>(rand_gen, node_weight_range));
            for (Id node = 1; node < node_cnt; ++node) {
                NodeWeight node_weight = 
                    gen_rand_in_range<NodeWeight>(rand_gen, node_weight_range);
                graph.node_weight(node, node_weight);

                Id parent = gen_ran_in_range(rand_gen, std::make_pair(0, node));
                while (graph.adj_nodes(parent).size() > max_degree) {
                    parent = gen_ran_in_range(rand_gen, std::make_pair(0, node));
                }
                EdgeWeight parent_edge_weight =
                    gen_rand_in_range<EdgeWeight>(edge_weight_range);
                graph.edge_weight(node, parent, parent_edge_weight);
            }
            
            return graph;
        }

    template<typename Id, typename NodeWeight, typename EdgeWeight>
        graph::Graph<Id, NodeWeight, EdgeWeight> tree_pref_attach(
                Id node_cnt,
                std::pair<NodeWeight, NodeWeight> node_weight_range = {1, 2},
                std::pair<EdgeWeight, EdgeWeight> edge_weight_range = {1, 101},
                Id max_degree = std::numeric_limits<Id>::max(),
                long seed = 0
                ) {
            RandGen rand_gen(seed);

            graph::Graph<Id, NodeWeight, EdgeWeight> graph;
            graph.resize(node_cnt);
            graph.node_weight(0, 
                    gen_rand_in_range<NodeWeight>(rand_gen, node_weight_range));

            // Insert a second node since non-zero weights are needed for the
            // discrete_distribution.
            graph.node_weight(1,
                    gen_rand_in_range<NodeWeight>(rand_gen, node_weight_range));

            graph.edge_weight(1, 0,
                    gen_rand_in_range<EdgeWeight>(edge_weight_range));

            std::vector<Id> degree(node_cnt, 0);
            degree.at(0) = 1;
            degree.at(1) = 1; 
            for (Id node = 2; node < node_cnt; ++node) {
                NodeWeight node_weight = 
                    gen_rand_in_range<NodeWeight>(rand_gen, node_weight_range);
                graph.node_weight(node, node_weight);

                std::discrete_distribution<Id> dist(degree.cbegin(),
                        degree.cend() + node);
                Id parent = dist(rand_gen);
                while (graph.adj_nodes(parent).size() > max_degree) {
                    parent = dist(rand_gen); 
                }
                EdgeWeight parent_edge_weight =
                    gen_rand_in_range<EdgeWeight>(edge_weight_range);
                graph.edge_weight(node, parent, parent_edge_weight);
                degree.at(parent) += 1;
                degree.at(node) += 1;
            }

            return graph;
        }
}
