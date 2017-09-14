/**
 * @file GraphGen.hpp
 */
#pragma once

#include<cstdint>
#include<fstream>
#include<iostream>
#include<limits>
#include<memory>
#include<random>
#include<string>

#include "Graph.hpp"
#include "GraphIo.hpp"
#include "GraphUtils.hpp"

/**
 * This namespace contains a number of different graph generators.
 * Every graph generator inherits from the abstract class graphgen::IGraphGen.
 */
namespace graphgen {

    /**
     * The interface of a graph generator as an abstract class.
     */
    template<typename Id=int32_t, typename NodeWeight=int32_t, typename EdgeWeight=int32_t>
        struct IGraphGen {
            public:
                /**
                 * Virtual deconstructor to prevent memory leaks.
                 */
                virtual ~IGraphGen() {}

                /**
                 * Generate a graph with the graph generator using \p seed.
                 * @param seed The seed to use (default 0)
                 * @returns The generated graph.
                 */
                virtual graph::Graph<Id, NodeWeight, EdgeWeight> operator()(size_t seed=0) const=0;
        };

    /**
     * Generate a random number in the given right exclusive range.
     * @param rand_gen The random generator to use.
     * @param range The right exclusive range.
     * @returns A random number in the specified range.
     */
    template<typename T, typename RandGen>
        T gen_rand_in_range(
                RandGen& rand_gen, std::pair<T, T> range) {
            std::uniform_int_distribution<T> dist(range.first, range.second - 1);    
            return dist(rand_gen);
        }

    /**
     * Use the TreeRandAttach heuristic to construct a graph.
     */
    template<typename Id=int32_t, typename NodeWeight=int32_t,
        typename EdgeWeight=int32_t, typename RandGen=std::mt19937_64>
            struct TreeRandAttach : public IGraphGen<Id, NodeWeight, EdgeWeight> {
                public:
                    Id node_cnt;
                    Id max_degree;
                    std::pair<NodeWeight, NodeWeight> node_weight_range;
                    std::pair<EdgeWeight, EdgeWeight> edge_weight_range;

                /**
                 * Constructor
                 * TreeRandAttach starts with one node and attaches new nodes
                 * randomly as leafs until the desired node count is reached.
                 * @param node_cnt The desired node count.
                 * @param max_degree The maximum degree of any node in the tree.
                 * @param node_weight_range The right exclusive range from which
                 *  the node weights should be chosen (default [1,2)).
                 * @param edge_weight_range The right exclusive range from which
                 *  the edge weights should be chosen (default [1,101)).
                 */
                TreeRandAttach(
                        Id node_cnt,
                        Id max_degree = std::numeric_limits<Id>::max(),
                        std::pair<NodeWeight, NodeWeight> node_weight_range = {1, 2},
                        std::pair<EdgeWeight, EdgeWeight> edge_weight_range = {1, 101}
                        ) :
                    node_cnt(node_cnt), max_degree(max_degree),
                    node_weight_range(node_weight_range), edge_weight_range(edge_weight_range) {}

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

    /**
     * Use the TreePrefAttach heuristic to construct a graph.
     */
    template<typename Id=int32_t, typename NodeWeight=int32_t,
        typename EdgeWeight=int32_t, typename RandGen=std::mt19937_64>
            struct TreePrefAttach : public IGraphGen<Id, NodeWeight, EdgeWeight> {
                public:
                    Id node_cnt;
                    Id max_degree;
                    std::pair<NodeWeight, NodeWeight> node_weight_range;
                    std::pair<EdgeWeight, EdgeWeight> edge_weight_range;

                /**
                 * Constructor
                 * TreePrefAttach starts with one node and attaches new nodes
                 * randomly to an existing node with probility proportional to 
                 * the degree of the existing node.
                 *
                 * @param node_cnt The desired node count.
                 * @param max_degree The maximum degree of any node in the tree.
                 * @param node_weight_range The right exclusive range from which
                 *  the node weights should be chosen (default [1,2)).
                 * @param edge_weight_range The right exclusive range from which
                 *  the edge weights should be chosen (default [1,101)).
                 */
                TreePrefAttach(
                        Id node_cnt,
                        Id max_degree = std::numeric_limits<Id>::max(),
                        std::pair<NodeWeight, NodeWeight> node_weight_range = {1, 2},
                        std::pair<EdgeWeight, EdgeWeight> edge_weight_range = {1, 101}
                        ) :
                    node_cnt(node_cnt), max_degree(max_degree),
                    node_weight_range(node_weight_range), edge_weight_range(edge_weight_range) {}
                
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

    /**
     * Use the fat tree heuristic to construct a graph.
     */
    template<typename Id=int32_t, typename NodeWeight=int32_t,
        typename EdgeWeight=int32_t, typename RandGen=std::mt19937_64>
            struct TreeFat : public IGraphGen<Id, NodeWeight, EdgeWeight> {
                public:
                    Id node_cnt;
                    std::pair<Id, Id> child_cnt_range;
                    std::pair<NodeWeight, NodeWeight> node_weight_range;
                    std::pair<EdgeWeight, EdgeWeight> edge_weight_range;

                /**
                 * Constructor
                 * TreeFet starts with one node and chooses a random number \p x from
                 * the right exclusive range \p child_cnt_range.
                 * Then \p x nodes are attached to all nodes in the previous level.
                 * This is done until \p node_cnt is reached.
                 *
                 * @param node_cnt The desired node count.
                 * @param child_cnt_range The right exclusive range from which the
                 *  child counts should be chosen.
                 * @param node_weight_range The right exclusive range from which
                 *  the node weights should be chosen (default [1,2)).
                 * @param edge_weight_range The right exclusive range from which
                 *  the edge weights should be chosen (default [1,101)).
                 */
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
                                    if (++node >= this->node_cnt) {
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

    /**
     * Use the Barabasi-Albert model to construct a graph.
     * <a href="https://en.wikipedia.org/wiki/Barab%C3%A1si%E2%80%93Albert_model">
     * Barabasi-Albert model on Wikipedia</a> 
     */
    template<typename Id=int32_t, typename NodeWeight=int32_t,
        typename EdgeWeight=int32_t, typename RandGen=std::mt19937_64>
            struct GraphPrefAttach : public IGraphGen<Id, NodeWeight, EdgeWeight> {
                public:
                    Id node_cnt;
                    Id edge_cnt_p_node;
                    Id max_degree;
                    std::pair<NodeWeight, NodeWeight> node_weight_range;
                    std::pair<EdgeWeight, EdgeWeight> edge_weight_range;

                    /**
                     * Constructor
                     * This constructs a graph with \p node_cnt nodes using Barabasi-Albert model.
                     * <a href="https://en.wikipedia.org/wiki/Barab%C3%A1si%E2%80%93Albert_model">
                     * Barabasi-Albert model on Wikipedia</a> 
                     *
                     * @param node_cnt The desired node count.
                     * @param edge_cnt_p_node The number of edges which are created for each node.
                     * @param max_degree The maximum degree of a node.
                     * @param node_weight_range The right exclusive range from which
                     *  the node weights should be chosen (default [1,2)).
                     * @param edge_weight_range The right exclusive range from which
                     *  the edge weights should be chosen (default [1,101)).
                     */
                    GraphPrefAttach(
                            Id node_cnt,
                            Id edge_cnt_p_node,
                            Id max_degree = std::numeric_limits<Id>::max(),
                            std::pair<NodeWeight, NodeWeight> node_weight_range = {1, 2},
                            std::pair<EdgeWeight, EdgeWeight> edge_weight_range = {1, 101}
                            ) :
                        node_cnt(node_cnt),
                        edge_cnt_p_node(edge_cnt_p_node),
                        max_degree(max_degree),
                        node_weight_range(node_weight_range),
                        edge_weight_range(edge_weight_range) {}

                    graph::Graph<Id, NodeWeight, EdgeWeight> operator()(size_t seed=0) const override {
                        RandGen rand_gen(seed);

                        graph::Graph<Id, NodeWeight, EdgeWeight> graph(this->node_cnt);
                        for (Id node = 0; node < graph.node_cnt(); ++node) {
                            graph.node_weight(node, 
                                    gen_rand_in_range<NodeWeight, RandGen>(rand_gen, this->node_weight_range));
                        }

                        std::vector<Id> degree(graph.node_cnt(), 0);
                        Id degree_sum = 0;
                        for (Id node = 0; node < edge_cnt_p_node; ++node) {
                            graph.edge_weight(node, (node + 1) % edge_cnt_p_node, 
                                    gen_rand_in_range<EdgeWeight, RandGen>(rand_gen, this->edge_weight_range));
                            degree.at(node) += 1;
                            degree.at((node + 1) % edge_cnt_p_node) += 1;
                            degree_sum += 2;
                        }

                        for (Id from_node = this->edge_cnt_p_node; from_node < graph.node_cnt(); ++from_node) {
                            for (Id edge_idx = 0; edge_idx < this->edge_cnt_p_node;) {

                                std::uniform_int_distribution<Id> to_node_deg_dist(0, degree_sum);
                                Id to_node_deg = to_node_deg_dist(rand_gen);
                                Id to_node;
                                for (to_node = 0;; ++to_node) {
                                    if (to_node_deg <= degree[to_node]) {
                                        break;
                                    }
                                    to_node_deg -= degree[to_node];
                                }

                                bool const node_degs_l_max = degree.at(from_node) < this->max_degree &&
                                    degree.at(to_node) < this->max_degree;
                                bool const exists_edge = graph.exists_edge(from_node, to_node);
                                if (node_degs_l_max) {

                                    EdgeWeight edge_weight = gen_rand_in_range<EdgeWeight, RandGen>(
                                            rand_gen, this->edge_weight_range);
                                    graph.add_edge_weight(from_node, to_node, edge_weight);

                                    if (!exists_edge) {
                                        degree.at(from_node) += 1;
                                        degree.at(to_node) += 1;
                                        degree_sum += 1;
                                    }

                                    ++edge_idx;
                                }
                            }
                            degree_sum += degree[from_node];
                        }
                        return graph;
                    }
            };

    /**
     * Use the Erdos-Renyi model to construct a graph.
     * <a href="https://en.wikipedia.org/wiki/Erd%C5%91s%E2%80%93R%C3%A9nyi_model">
     * Erdos-Renyi model on Wikipedia</a> 
     */
    template<typename Id=int32_t, typename NodeWeight=int32_t,
        typename EdgeWeight=int32_t, typename RandGen=std::mt19937_64>
            struct GraphEdgeProb : public IGraphGen<Id, NodeWeight, EdgeWeight> {
                public:
                    Id node_cnt;
                    double edge_prob;
                    Id max_degree;
                    std::pair<NodeWeight, NodeWeight> node_weight_range;
                    std::pair<EdgeWeight, EdgeWeight> edge_weight_range;

                    /**
                     * Constructor
                     * This constructs a graph with \p node_cnt nodes using Erdos-Renyi model.
                     * <a href="https://en.wikipedia.org/wiki/Erd%C5%91s%E2%80%93R%C3%A9nyi_model">
                     * Erdos-Renyi model on Wikipedia</a> 
                     *
                     * @param node_cnt The desired node count.
                     * @param edge_prob The probability that an edge exists.
                     * @param max_degree The maximum degree of a node.
                     * @param node_weight_range The right exclusive range from which
                     *  the node weights should be chosen (default [1,2)).
                     * @param edge_weight_range The right exclusive range from which
                     *  the edge weights should be chosen (default [1,101)).
                     */
                    GraphEdgeProb(
                            Id node_cnt,
                            double edge_prob,
                            Id max_degree = std::numeric_limits<Id>::max(),
                            std::pair<NodeWeight, NodeWeight> node_weight_range = {1, 2},
                            std::pair<EdgeWeight, EdgeWeight> edge_weight_range = {1, 101}
                            ) :
                        node_cnt(node_cnt),
                        edge_prob(edge_prob),
                        max_degree(max_degree),
                        node_weight_range(node_weight_range),
                        edge_weight_range(edge_weight_range) {}

                    graph::Graph<Id, NodeWeight, EdgeWeight> operator()(size_t seed=0) const override {
                        RandGen rand_gen(seed);

                        graph::Graph<Id, NodeWeight, EdgeWeight> graph(this->node_cnt);
                        for (Id node = 0; node < graph.node_cnt(); ++node) {
                            graph.node_weight(node, 
                                    gen_rand_in_range<NodeWeight, RandGen>(rand_gen, this->node_weight_range));
                        }

                        for (Id from_node = 0; from_node < graph.node_cnt(); ++from_node) {
                            for (Id to_node = from_node + 1; to_node < graph.node_cnt(); ++to_node) {
                                std::bernoulli_distribution edge_prob_dist(this->edge_prob);
                                if (edge_prob_dist(rand_gen)) {
                                    EdgeWeight edge_weight = gen_rand_in_range<EdgeWeight, RandGen>(
                                            rand_gen, this->edge_weight_range);
                                    graph.edge_weight(from_node, to_node, edge_weight);
                                }
                            }
                        }

                        return graph;
                    }
            };

    /**
     * Use MST to convert a graph into a tree.
     */
    template<typename Id=int32_t, typename NodeWeight=int32_t,
        typename EdgeWeight=int32_t>
            struct Mst : public IGraphGen<Id, NodeWeight, EdgeWeight> {
                public:
                    std::shared_ptr<IGraphGen<Id, NodeWeight, EdgeWeight>> graph_gen;

                    /**
                     * Compute the MST for a graph.
                     * When the graph generator is called then the MST is computed for the 
                     * graph returned by \p graph_gen.
                     * @param graph_gen The graph generator to use.
                     *
                     * @see graph::mst()
                     */
                    Mst(std::shared_ptr<IGraphGen<Id, NodeWeight, EdgeWeight>> const& graph_gen) :
                        graph_gen(graph_gen) {}

                    graph::Graph<Id, NodeWeight, EdgeWeight> operator()(size_t seed=0) const override {
                        return graph::mst<Id, NodeWeight, EdgeWeight>((*graph_gen)(seed));
                    }
            };

    /**
     * Use RST to convert a graph into a tree.
     */
    template<typename Id=int32_t, typename NodeWeight=int32_t,
        typename EdgeWeight=int32_t, typename RandGen=std::mt19937_64>
            struct Rst : public IGraphGen<Id, NodeWeight, EdgeWeight> {
                public:
                    std::shared_ptr<IGraphGen<Id, NodeWeight, EdgeWeight>> graph_gen;
                    size_t rst_seed;

                    /**
                     * Compute the RST for a graph.
                     * When the graph generator is called then the RST is computed for the 
                     * graph returned by \p graph_gen.
                     * @param graph_gen The graph generator to use.
                     * @param rst_seed The seed for generating the rst (default 0).
                     *
                     * @see graph::rst()
                     */
                    Rst(std::shared_ptr<IGraphGen<Id, NodeWeight, EdgeWeight>> const& graph_gen,
                            size_t rst_seed=0) :
                        graph_gen(graph_gen), rst_seed(rst_seed) {}

                    graph::Graph<Id, NodeWeight, EdgeWeight> operator()(size_t seed=0) const override {
                        return graph::rst<Id, NodeWeight, EdgeWeight, RandGen>(
                                (*graph_gen)(seed), this->rst_seed);
                    }
            };

    /**
     * Uses heavy edge matching to contract a graph to n nodes.
     * @see graph::heavy_edge_matching()
     * @see graph::contract_to_n_nodes()
     */
    template<typename Id=int32_t, typename NodeWeight=int32_t,
        typename EdgeWeight=int32_t, typename RandGen=std::mt19937_64>
            struct ContractToN : public IGraphGen<Id, NodeWeight, EdgeWeight> {
                public:
                    std::shared_ptr<IGraphGen<Id, NodeWeight, EdgeWeight>> graph_gen;
                    Id node_count;
                    size_t matching_seed;

                    /**
                     * Construct a graph to \p node_count nodes using Heavy Edge Matching.
                     *
                     * @param graph_gen The graph generator to use.
                     * @param node_count The number of nodes after contraction.
                     * @param matching_seed The seed to generate the matching (default 0).
                     * @see graph::heavy_edge_matching()
                     * @see graph::contract_to_n_nodes()
                     */
                    ContractToN(
                            std::shared_ptr<IGraphGen<Id, NodeWeight, EdgeWeight>> const& graph_gen,
                            Id node_count,
                            size_t matching_seed=0
                            ) : 
                        graph_gen(graph_gen), node_count(node_count), matching_seed(matching_seed) {}

                    graph::Graph<Id, NodeWeight, EdgeWeight> operator()(size_t seed=0) const override {
                        return graph::contract_to_n_nodes(
                                (*graph_gen)(seed), this->node_count, this->matching_seed);
                    }
            };

    /**
     * This just returns the graphs itself.
     * Returns the i-th graph for seed i.
     */
    template<typename Id=int32_t, typename NodeWeight=int32_t, typename EdgeWeight=int32_t>
        struct GraphId : public IGraphGen<Id, NodeWeight, EdgeWeight> {
            public:
                std::vector<graph::Graph<Id, NodeWeight, EdgeWeight>> graphs;

                /**
                 * Constructor from a single graph.
                 * @param graph The graph.
                 */
                GraphId(graph::Graph<Id, NodeWeight, EdgeWeight> const& graph) : 
                    graphs({graph}) {}

                /**
                 * Constructor from multiple graph.
                 * @param graphs The graphs.
                 */
                GraphId(std::vector<graph::Graph<Id, NodeWeight, EdgeWeight>> graphs) :
                    graphs(graphs) {}

                graph::Graph<Id, NodeWeight, EdgeWeight> operator()(size_t seed=0) const override {
                    return graphs.at(seed);
                }
        };

    /**
     * Read in graphs from file or stdin.
     * Returns the i-th graph from the file for seed i.
     */
    template<typename Id=int32_t, typename NodeWeight=int32_t, typename EdgeWeight=int32_t>
        struct FromFile : public IGraphGen<Id, NodeWeight, EdgeWeight> {
            private:
                std::vector<graph::Graph<Id, NodeWeight, EdgeWeight>> graphs;
                std::string filename;
                bool do_read_on_demand;

            public:
                /**
                 * Read in \p graph_cnt graphs from file \p filename.
                 * @param filename The file to use "-" for stdin.
                 * @param graph_cnt The number of graphs to read from the file.
                 * @param do_read_on_demand Indicates whether the graphs should be read only when 
                 *  needed to save memory. This only works for files. Default: false.
                 */
                FromFile(std::string filename, size_t graph_cnt=1, bool do_read_on_demand=false) :
                    filename(filename), do_read_on_demand(do_read_on_demand) {
                        if (!do_read_on_demand) {
                            if (filename == std::string("-")) {
                                for (size_t graph_idx = 0; graph_idx < graph_cnt; ++graph_idx) {
                                    graphs.emplace_back();
                                    std::cin >> graphs.back();
                                }
                            } else {
                                std::ifstream file(filename);
                                for (size_t graph_idx = 0; graph_idx < graph_cnt; ++graph_idx) {
                                    graphs.emplace_back();
                                    file >> graphs.back();
                                }
                            }
                        }
                    }

                graph::Graph<Id, NodeWeight, EdgeWeight> operator()(size_t seed=0) const override {
                    if (do_read_on_demand) {
                        graph::Graph<Id, NodeWeight, EdgeWeight> graph;
                        if (filename == std::string("-")) {
                            for (size_t graph_idx = 0; graph_idx <= seed; ++graph_idx) {
                                std::cin >> graph;
                            }
                        } else {
                            std::ifstream file(filename);
                            for (size_t graph_idx = 0; graph_idx <= seed; ++graph_idx) {
                                file >> graph;
                            }
                        }
                        return graph;
                    } else {
                        return graphs.at(seed);
                    }
                }
        };

    /**
     * Contract infinite edges of a given graph.
     * The maximum value of \p EdgeWeight is treated as infinity.
     * This is used to contract to deal with the infinity edges of the hierarchical 
     * decomposition.
     */
    template<typename Id=int32_t, typename NodeWeight=int32_t, typename EdgeWeight=int32_t>
        struct ContractInfEdges : public IGraphGen<Id, NodeWeight, EdgeWeight> {
            public:
                std::shared_ptr<IGraphGen<Id, NodeWeight, EdgeWeight>> graph_gen;
                EdgeWeight infty;

                /** 
                 * Constructor.
                 * @param graph_gen The graph generator.
                 * @param infty The value to treat as infinity. 
                 *  Default: <code>std::numeric_limits<EdgeWeight>::max()</code>.
                 */
                ContractInfEdges(
                        std::shared_ptr<IGraphGen<Id, NodeWeight, EdgeWeight>> const& graph_gen,
                        EdgeWeight infty = std::numeric_limits<EdgeWeight>::max()
                        ) : graph_gen(graph_gen), infty(infty) {}

                graph::Graph<Id, NodeWeight, EdgeWeight> operator()(size_t seed=0) const override {
                    using Graph = graph::Graph<Id, NodeWeight, EdgeWeight>;
                    Graph graph = (*this->graph_gen)(seed);
                    typename Graph::Matching matching;
                    do {
                        matching = typename Graph::Matching();
                        std::vector<bool> is_matched(graph.node_cnt());
                        for (Id node = 0; node < graph.node_cnt(); ++node) {
                            for (auto const& edge : graph.inc_edges(node)) {
                                if (edge.second == this->infty &&
                                        !is_matched.at(node) && !is_matched.at(edge.first)) {
                                    matching.emplace_back(node, edge.first);
                                    is_matched[node] = true;
                                    is_matched[edge.first] = true;
                                }
                            }
                        } 
                        graph.contract_edges(matching);
                    } while (matching.size() > 0);

                    return graph;
                }
        };

}
