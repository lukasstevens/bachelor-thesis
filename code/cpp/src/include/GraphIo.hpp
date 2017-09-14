/** 
 * @file GraphIo.hpp 
 * This file contains all utility functions to read and write graphs.
 * Note that the overloads for the inputstream operators are not 
 * namespaced.
 * @see graph::Graph
 */
#pragma once

#include<cstdint>
#include<iostream>
#include<sstream>

#include "Graph.hpp"

/**
 * Reads a graph in CSR format from \p is.
 * The format is described in the metis manual in \p deps/metis/manual.
 * @param is The inputstream.
 * @param graph The graph to write to.
 * @returns The input stream.
 */
template<typename Id, typename NodeWeight, typename EdgeWeight>
std::istream& operator>>(std::istream& is, graph::Graph<Id, NodeWeight, EdgeWeight>& graph) {
    Id node_cnt;
    Id edge_cnt;

    bool has_node_sizes = false;
    bool has_node_weights = false;
    bool has_edge_weights = false;
    std::vector<bool*> fmt({&has_node_sizes, &has_node_weights, &has_edge_weights});

    std::string line;
    while (std::getline(is, line) && (line.front() == '%' || line.empty())) {
        // Skip lines with comments
    }

    std::istringstream line_buffer(line);
    if (!(line_buffer >> node_cnt >> edge_cnt)) {
        throw std::invalid_argument("Node count or edge count missing");
    } 

    std::string fmt_string;
    if (line_buffer >> fmt_string) {
        fmt_string.insert(0, 3 - fmt_string.size(), '0');
        for (size_t i = 0; i < fmt_string.size(); ++i) {
            *fmt.at(i) = '1' == fmt_string.at(i);
        }
        if (has_node_sizes) {
            throw std::invalid_argument("Node sizes not supported.");
        }
    }

    int ncon;
    if ((line_buffer >> ncon) && ncon != 1) {
        throw std::invalid_argument("Multiple node weights not allowed.");
    }

    // Delete all nodes which were in the graph before.
    graph.resize(0);
    graph.resize(static_cast<size_t>(node_cnt));

    for (Id curr_node = 0; curr_node < node_cnt && std::getline(is, line); ++curr_node) {
        if (line.front() == '%') {
            --curr_node;
            continue;
        }

        line_buffer.str(line);
        line_buffer.clear();

        NodeWeight curr_node_weight = 1;
        if(has_node_weights && !(line_buffer >> curr_node_weight)) {
            std::ostringstream error_msg("Node weight missing at node ",
                    std::ios_base::app);
            error_msg << curr_node << '.'; 
            throw std::invalid_argument(error_msg.str());
        }
        graph.node_weight(curr_node, curr_node_weight);

        Id to_node;
        EdgeWeight edge_weight = 1;
        while (line_buffer >> to_node) {
            if (has_edge_weights && !(line_buffer >> edge_weight)) {
                std::ostringstream error_msg("Edge weight missing at edge from ",
                        std::ios_base::app);
                error_msg << curr_node << " to " << to_node << '.';
                throw std::invalid_argument(error_msg.str());
            }
            graph.edge_weight(curr_node, to_node, edge_weight);
            graph.edge_weight(to_node, curr_node, edge_weight);
        }
    }

    return is;
}

/**
 * Writes in CSR format to \p os.
 * The format is described in the metis manual in \p deps/metis/manual.
 * @param os The outputstream.
 * @param graph The graph to output.
 * @returns The output stream.
 */
template<typename Id, typename NodeWeight, typename EdgeWeight>
std::ostream& operator<<(std::ostream& os, graph::Graph<Id, NodeWeight, EdgeWeight> const& graph) {
    os << graph.node_cnt() << " " << graph.edge_cnt() << " 011" << '\n';

    for(Id curr_node = 0; curr_node < graph.node_cnt(); ++curr_node) {
        os << graph.node_weight(curr_node);
        for (Id to_node : graph.adj_nodes(curr_node)) {
            os << " " << to_node << " " << graph.edge_weight(curr_node, to_node);
        }
        os << '\n';
    }

    return os;
}

namespace graphio {

    /**
     * A helper struct to print the graph as graphviz.
     */
    template<typename Id=int32_t, typename NodeWeight=int32_t, typename EdgeWeight=int32_t>
        struct PrintGraphviz {
            graph::Graph<Id, NodeWeight, EdgeWeight> const& graph;
            bool const is_zero_indexed;
            std::vector<Id> const partition;

            /**
             * Construct a graphviz printer.
             * @param graph The graph to print.
             * @param partition Optionally specify a partition.
             *  This will color the nodes accordingly.
             * @param is_zero_indexed Specifies whether to start node ids at 0 or 1.
             */
            PrintGraphviz(graph::Graph<Id, NodeWeight, EdgeWeight> const& graph, 
                    std::vector<Id> partition={}, bool is_zero_indexed=true)
                : graph(graph), is_zero_indexed(is_zero_indexed), partition(partition) {}
        };
}

/**
 * Print a graphviz printer.
 * @param os The output stream.
 * @param print The printer.
 * @returns The output stream.
 */
template<typename Id, typename NodeWeight, typename EdgeWeight>
std::ostream& operator<<(std::ostream& os,
        graphio::PrintGraphviz<Id, NodeWeight, EdgeWeight> const& print) {
    os << "graph G {\n";
    for (Id node = 0; node < print.graph.node_cnt(); ++node) {
        Id node_label = (print.is_zero_indexed) ? node : node + 1;
        os << "\t" << node << "[label=\"";
        os << "id: " << node_label << ", ";
        os << "w: " << print.graph.node_weight(node) << ", ";
        os << "r:";
        for (auto const& id : print.graph.node_repr(node)) {
            os << " " << id;
        }
        os << "\"";
        if (print.partition.size() != 0) {
            os << ", style=filled, color=\"/spectral9/";
            os << (print.partition.at(node) + 1) << "\"";
        }
        os << "];\n";
        for (auto const& inc_edge : print.graph.inc_edges(node)) {
            if (inc_edge.first >=  node) {
                os << "\t" << node << " -- " << inc_edge.first;
                os << "[label=\"" << inc_edge.second << "\"];\n";
            }
        }
    }
    os << "}" << std::endl;
    return os;
}

namespace graphio {
    
    /**
     * Read a graph in the old tree format.
     */
    template<typename Id=int32_t, typename NodeWeight=int32_t, typename EdgeWeight=int32_t>
        struct ReadTreeFormat {
            graph::Graph<Id, NodeWeight, EdgeWeight>& graph;
            bool const is_zero_indexed;

            /**
             * Constructor.
             * @param graph The graph to read into.
             * @param is_zero_indexed Indicates whether the nodes in the input are indexed
             *  starting from 0 or from 1.
             */
            ReadTreeFormat(graph::Graph<Id, NodeWeight, EdgeWeight>& graph, 
                    bool is_zero_indexed=false) :
                graph(graph), is_zero_indexed(is_zero_indexed) {}
        };
}

/**
 * Reads a grap in the old tree format.
 * @param is The inputstream.
 * @param read The reader.
 * @returns The input stream.
 */
template<typename Id, typename NodeWeight, typename EdgeWeight>
std::istream& operator>>(std::istream& is, graphio::ReadTreeFormat<Id, NodeWeight, EdgeWeight>& read) {
    Id node_cnt;
    // root_id will be ignored in the graph format.
    Id root_id;
    is >> node_cnt >> root_id;
    // Delete everything in the graph first.
    read.graph.resize(0);
    read.graph.resize(node_cnt);
    for (Id edge_idx = 0; edge_idx < node_cnt; ++edge_idx) {
        Id from_node;
        Id to_node;
        EdgeWeight weight;
        is >> from_node >> to_node >> weight;
        if (!read.is_zero_indexed) {
            from_node -= 1;
            to_node -= 1;
        }
        read.graph.edge_weight(from_node, to_node, weight);
    }
    return is;
}

namespace graphio {
    /**
     * Reader for graphs in the format of the Stanford Large Network Collection.
     */
    template<typename Id=int32_t, typename NodeWeight=int32_t, typename EdgeWeight=int32_t>
        struct ReadSnapFormat {
            graph::Graph<Id, NodeWeight, EdgeWeight>& graph;

            /**
             * Construct a reader.
             * @param graph The graph to read into.
             */
            ReadSnapFormat(graph::Graph<Id, NodeWeight, EdgeWeight>& graph) :
                graph(graph) {}
        };
}

/**
 * Read a graph in the snap format.
 * @param is The inputstream.
 * @param read The reader.
 * @returns The inputstream.
 */
template<typename Id, typename NodeWeight, typename EdgeWeight>
std::istream& operator>>(std::istream& is, graphio::ReadSnapFormat<Id, NodeWeight, EdgeWeight>& read) {
    Id node_cnt;
    Id edge_cnt;
    is >> node_cnt >> edge_cnt;
    // Delete everything in the graph first.
    std::unordered_map<Id, Id> node_labels;
    Id node_label = 0;
    read.graph = graph::Graph<Id, NodeWeight, EdgeWeight>(node_cnt);
    for (Id edge_idx = 0; edge_idx < edge_cnt; ++edge_idx) {
        Id from_node;
        Id to_node;
        is >> from_node >> to_node;
        if (node_labels.find(from_node) == node_labels.cend()) {
            node_labels[from_node] = node_label;
            node_label += 1;
        }
        if (node_labels.find(to_node) == node_labels.cend()) {
            node_labels[to_node] = node_label;
            node_label += 1;
        }
        read.graph.edge_weight(
                node_labels.at(from_node), node_labels.at(to_node), 1);
    }

    return is;
}
