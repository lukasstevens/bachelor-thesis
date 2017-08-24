#include<random>
#include<limits.h>

#include<Graph.hpp>

namespace graphgen {

    template<typename Id=int, typename NodeWeight=int, typename EdgeWeight=int>
        struct IGraphGen {
            public:
                virtual ~IGraphGen() {}
                virtual graph::Graph<Id, NodeWeight, EdgeWeight> operator()(size_t seed=0) const=0;
        };

    template<typename T, typename RandGen>
        T gen_rand_in_range(
                RandGen& rand_gen, std::pair<T, T> range) {
            std::uniform_int_distribution<T> dist(range.first, range.second - 1);    
            return dist(rand_gen);
        }

    template<typename Id=int, typename NodeWeight=int,
        typename EdgeWeight=int, typename RandGen=std::mt19937_64>
            struct TreeRandAttach : public IGraphGen<Id, NodeWeight, EdgeWeight> {
                public:
                    Id node_cnt;
                    std::pair<NodeWeight, NodeWeight> node_weight_range;
                    std::pair<EdgeWeight, EdgeWeight> edge_weight_range;
                    Id max_degree;

                TreeRandAttach(
                        Id node_cnt,
                        Id max_degree = std::numeric_limits<Id>::max(),
                        std::pair<NodeWeight, NodeWeight> node_weight_range = {1, 2},
                        std::pair<EdgeWeight, EdgeWeight> edge_weight_range = {1, 101}
                        ) :
                    node_cnt(node_cnt), node_weight_range(node_weight_range),
                    edge_weight_range(edge_weight_range), max_degree(max_degree) {}

                graph::Graph<Id, NodeWeight, EdgeWeight> operator()(size_t seed=0) const override {
                    RandGen rand_gen(seed);

                    graph::Graph<Id, NodeWeight, EdgeWeight> graph;
                    graph.resize(this->node_cnt);
                    graph.node_weight(0, 
                            gen_rand_in_range<NodeWeight, RandGen>(rand_gen, this->node_weight_range));
                    for (Id node = 1; node < this->node_cnt; ++node) {
                        NodeWeight node_weight = 
                            gen_rand_in_range<NodeWeight, RandGen>(rand_gen, this->node_weight_range);
                        graph.node_weight(node, node_weight);

                        Id parent = gen_rand_in_range<Id, RandGen>(rand_gen, std::make_pair(0, node));
                        while (graph.adj_nodes(parent).size() > this->max_degree) {
                            parent = gen_rand_in_range<Id, RandGen>(rand_gen, std::make_pair(0, node));
                        }
                        EdgeWeight parent_edge_weight =
                            gen_rand_in_range<EdgeWeight, RandGen>(rand_gen, this->edge_weight_range);
                        graph.edge_weight(node, parent, parent_edge_weight);
                    }

                    return graph;
                }
            };

    template<typename Id=int, typename NodeWeight=int,
        typename EdgeWeight=int, typename RandGen=std::mt19937_64>
            struct TreePrefAttach : public IGraphGen<Id, NodeWeight, EdgeWeight> {
                public:
                    Id node_cnt;
                    std::pair<NodeWeight, NodeWeight> node_weight_range;
                    std::pair<EdgeWeight, EdgeWeight> edge_weight_range;
                    Id max_degree;

                TreePrefAttach(
                        Id node_cnt,
                        Id max_degree = std::numeric_limits<Id>::max(),
                        std::pair<NodeWeight, NodeWeight> node_weight_range = {1, 2},
                        std::pair<EdgeWeight, EdgeWeight> edge_weight_range = {1, 101}
                        ) :
                    node_cnt(node_cnt), node_weight_range(node_weight_range),
                    edge_weight_range(edge_weight_range), max_degree(max_degree) {}
                
                graph::Graph<Id, NodeWeight, EdgeWeight> operator()(size_t seed=0) const override {
                    RandGen rand_gen(seed);

                    graph::Graph<Id, NodeWeight, EdgeWeight> graph;
                    graph.resize(this->node_cnt);
                    graph.node_weight(0, 
                            gen_rand_in_range<NodeWeight>(rand_gen, this->node_weight_range));

                    // Insert a second node since non-zero weights are needed for the
                    // discrete_distribution.
                    graph.node_weight(1,
                            gen_rand_in_range<NodeWeight>(rand_gen, this->node_weight_range));

                    graph.edge_weight(1, 0,
                            gen_rand_in_range<EdgeWeight>(rand_gen, this->edge_weight_range));

                    std::vector<Id> degree(this->node_cnt, 0);
                    degree.at(0) = 1;
                    degree.at(1) = 1; 
                    for (Id node = 2; node < this->node_cnt; ++node) {
                        NodeWeight node_weight = 
                            gen_rand_in_range<NodeWeight>(rand_gen, this->node_weight_range);
                        graph.node_weight(node, node_weight);

                        std::discrete_distribution<Id> dist(degree.cbegin(),
                                degree.cbegin() + node);
                        Id parent = dist(rand_gen);
                        while (graph.adj_nodes(parent).size() > this->max_degree) {
                            parent = dist(rand_gen); 
                        }
                        EdgeWeight parent_edge_weight =
                            gen_rand_in_range<EdgeWeight>(rand_gen, this->edge_weight_range);
                        graph.edge_weight(node, parent, parent_edge_weight);
                        degree.at(parent) += 1;
                        degree.at(node) += 1;
                    }

                    return graph;
                }

            };

    template<typename Id=int, typename NodeWeight=int,
        typename EdgeWeight=int, typename RandGen=std::mt19937_64>
            struct TreeFat : public IGraphGen<Id, NodeWeight, EdgeWeight> {
                public:
                    Id node_cnt;
                    std::pair<Id, Id> child_cnt_range;
                    std::pair<NodeWeight, NodeWeight> node_weight_range;
                    std::pair<EdgeWeight, EdgeWeight> edge_weight_range;

                    TreeFat(
                            Id node_cnt,
                            std::pair<Id, Id> child_cnt_range={1, std::numeric_limits<Id>::max()},
                            std::pair<NodeWeight, NodeWeight> node_weight_range = {1, 2},
                            std::pair<EdgeWeight, EdgeWeight> edge_weight_range = {1, 101}
                           ) :
                        node_cnt(node_cnt), child_cnt_range(child_cnt_range),
                        node_weight_range(node_weight_range), edge_weight_range(edge_weight_range) {}

                    graph::Graph<Id, NodeWeight, EdgeWeight> operator()(size_t seed=0) const override {
                        RandGen rand_gen(seed);

                        graph::Graph<Id, NodeWeight, EdgeWeight> graph;
                        graph.resize(this->node_cnt);
                        graph.node_weight(0,
                                gen_rand_in_range<NodeWeight>(rand_gen, this->node_weight_range));
                        std::vector<Id> last_level({0});
                        std::vector<Id> this_level;
                        Id node = 1;
                        while (true) {
                            Id level_degree = gen_rand_in_range<Id>(rand_gen, this->child_cnt_range);
                            for (auto const& parent : last_level) {
                                for (Id child_idx = 0; child_idx < level_degree; ++child_idx) {
                                    graph.node_weight(node,
                                            gen_rand_in_range<NodeWeight>(rand_gen, this->node_weight_range));
                                    graph.edge_weight(node, parent,
                                            gen_rand_in_range<EdgeWeight>(rand_gen, this->edge_weight_range));
                                    this_level.push_back(node);
                                    ++node;
                                    if (node >= this->node_cnt) {
                                        return graph;
                                    }
                                }
                            }
                            std::swap(this_level, last_level);
                            this_level = std::vector<Id>();
                        }
                        return graph;
                    }
            };
}
