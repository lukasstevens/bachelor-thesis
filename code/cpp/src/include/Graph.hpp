#include<algorithm>
#include<iostream>
#include<limits>
#include<set>
#include<stdexcept>
#include<string>
#include<sstream>
#include<unordered_map>
#include<vector>

#include<metis.h>

namespace graph {
    struct MetisGraph {
        public:
            idx_t nvtxs;            
            std::vector<idx_t> xadj;
            std::vector<idx_t> adjncy;
            std::vector<idx_t> vwgt;
            std::vector<idx_t> adjwgt;

            MetisGraph(
                    std::vector<idx_t> xadj,
                    std::vector<idx_t> adjncy,
                    std::vector<idx_t> vwgt,
                    std::vector<idx_t> adjwgt) :
                nvtxs(static_cast<idx_t>(vwgt.size())),
                xadj(xadj),
                adjncy(adjncy),
                vwgt(vwgt),
                adjwgt(adjwgt) {}
    };

    template<typename Id=int, typename NodeWeight=int, typename EdgeWeight=int>
        struct Graph {
            private:
                using NodeSet = std::set<Id>;

                std::unordered_map<Id, std::unordered_map<Id, EdgeWeight>> adjncy;
                std::vector<NodeSet> vrepr;
                std::vector<NodeWeight> vwgt;

                Graph(
                        std::unordered_map<Id, std::unordered_map<Id, EdgeWeight>> adjncy,
                        std::vector<NodeWeight> vwgt
                     ) : adjncy(adjncy), vwgt(vwgt)
                {
                    for(Id i = 0; static_cast<std::size_t>(i) < adjncy.size(); ++i) {
                        vrepr.at(static_cast<std::size_t>(i)).insert(i);
                    }
                }

            public:

                Graph(std::vector<NodeWeight> vwgt) : 
                    Graph(
                            std::unordered_map<Id, std::unordered_map<Id, EdgeWeight>>(),
                            vwgt
                         ) {}

                Graph(Id node_cnt) : 
                    Graph(std::vector<NodeWeight>(static_cast<std::size_t>(node_cnt), 1)) {}

                Graph() = default;

                Id node_cnt() const {
                    if (vwgt.size() <= std::numeric_limits<Id>::max()) {
                        return static_cast<Id>(vwgt.size());
                    } else {
                        throw std::overflow_error("The node count is too large for the type Id");
                    }
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

                EdgeWeight edge_weight(Id from_node, Id to_node) const {
                    return this->adjncy.at(from_node).at(to_node);
                }

                void edge_weight(Id from_node, Id to_node, EdgeWeight weight) {
                    this->adjncy.at(from_node)[to_node] = weight;
                    this->adjncy.at(to_node)[from_node] = weight;
                }

                std::vector<Id> adj_nodes(Id node) const {
                    std::vector<Id> adj_nodes;
                    if (this->adjncy.find(node) != this->adjncy.end()) {
                        adj_nodes.reserve(this->adjncy.at(node).size());
                        for (auto const& edge : this->adjncy.at(node)) {
                            adj_nodes.push_back(edge.first);
                        }
                    }
                    return adj_nodes;
                }

                std::vector<std::pair<Id, EdgeWeight>> inc_edges(Id node) const {
                    if (this->adjncy.find(node) != this->adjncy.end()) {
                        return std::vector<std::pair<Id, EdgeWeight>>(
                                adjncy.at(node).cbegin(), adjncy.at(node).cend());
                    } else {
                        return std::vector<std::pair<Id, EdgeWeight>>();
                    }
                }

                void resize(Id node_cnt) {
                    auto n = static_cast<std::size_t>(node_cnt);
                    this->vrepr.resize(n);
                    for (Id i = 0; i < node_cnt; ++i) {
                        if (vrepr.at(static_cast<std::size_t>(i)).empty()) {
                            vrepr.at(static_cast<std::size_t>(i)).insert(i);
                        }
                    }
                    this->vwgt.resize(n, 1);

                    std::unordered_map<Id, std::unordered_map<Id, EdgeWeight>> new_adjncy;
                    for (Id i = 0; i < node_cnt; ++i) {
                        if (new_adjncy.find(i) != this->adjncy.end()) {
                            auto const& i_adjncy = new_adjncy.at(i);
                            for (auto const& edge : i_adjncy) {
                                if (edge.first < n) {
                                    new_adjncy[i][edge.first] = edge.second;
                                    new_adjncy[edge.first][i] = edge.second;
                                }
                            }
                        }
                    }
                    this->adjncy = new_adjncy;
                }

                MetisGraph to_metis_graph() const {
                    Id const node_cnt = this->node_cnt();
                    Id const edge_cnt = this->edge_cnt();

                    std::vector<idx_t> metis_vwgt(this->vwgt.cbegin(), this->vwgt.cend());
                    std::vector<idx_t> metis_xadj(static_cast<std::size_t>(node_cnt + 1));
                    std::vector<idx_t> metis_adjncy;
                    metis_adjncy.reserve(static_cast<std::size_t>(edge_cnt));
                    std::vector<idx_t> metis_adjwgt;
                    metis_adjwgt.reserve(static_cast<std::size_t>(edge_cnt));

                    for (Id curr_node = 0; curr_node < node_cnt; ++curr_node) {
                        std::size_t const curr_node_st = static_cast<std::size_t>(curr_node);
                        metis_xadj.at(curr_node_st + 1) = metis_xadj.at(curr_node_st);
                        for (auto const& inc_edge : this->adjncy.at(curr_node)) {
                            metis_adjncy.push_back(inc_edge.first); 
                            metis_adjwgt.push_back(inc_edge.second);
                            metis_xadj.at(curr_node_st + 1) += 1;
                        }
                    }
                    return MetisGraph(metis_xadj, metis_adjncy, metis_vwgt, metis_adjwgt);
                }
        };

}

template<typename Id, typename NodeWeight, typename EdgeWeight>
std::istream& operator>>(std::istream& is, graph::Graph<Id, NodeWeight, EdgeWeight>& graph) {
    Id node_cnt;
    Id edge_cnt;

    bool has_node_sizes = false;
    bool has_node_weights = false;
    bool has_edge_weights = false;
    std::vector<bool*> fmt({&has_node_sizes, &has_node_weights, &has_edge_weights});

    std::string line;
    while (std::getline(is, line) && line.front() == '%') {
        // Skip lines with comments
    }

    std::istringstream line_buffer(line);
    if (!(line_buffer >> node_cnt >> edge_cnt)) {
        throw std::invalid_argument("Node count or edge count missing");
    } 

    std::string fmt_string;
    if (line_buffer >> fmt_string) {
        fmt_string.insert(0, 3 - fmt_string.size(), '0');
        for (std::size_t i = 0; i < fmt_string.size(); ++i) {
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
    graph.resize(static_cast<std::size_t>(node_cnt));

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
