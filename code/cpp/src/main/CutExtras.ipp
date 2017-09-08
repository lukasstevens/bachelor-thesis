namespace cut {

    template<typename Id, typename NodeWeight, typename EdgeWeight>
        std::pair<size_t, size_t> Tree<Id, NodeWeight, EdgeWeight>::get_node_idx(Id node_id) const {
            for (size_t lvl_idx = 0; lvl_idx < this->levels.size(); ++lvl_idx) {
                for (size_t node_idx = 0; node_idx < this->levels[lvl_idx].size(); ++node_idx) {
                    if (this->levels[lvl_idx][node_idx].id == node_id) {
                        return std::make_pair(lvl_idx, node_idx);
                    }
                }
            }
            return std::make_pair(std::numeric_limits<size_t>::max(), 
                    std::numeric_limits<size_t>::max());
        }

    template<typename Id, typename NodeWeight, typename EdgeWeight>
        std::string Tree<Id, NodeWeight, EdgeWeight>::as_graphviz() const {

            std::stringstream stream;
            stream << "digraph tree {\n";
            stream << "\t" << this->levels[0][0].id << "[label=\"" << this->levels[0][0].id;
            stream << "(w:" << this->levels[0][0].parent_edge_weight << ")\"]\n";
            for (size_t lvl_idx = this->levels.size() - 1; lvl_idx > 0; --lvl_idx) {
                for (size_t node_idx = 0; node_idx < this->levels[lvl_idx].size(); ++node_idx) {
                    Tree::Node const& node = this->levels[lvl_idx][node_idx];
                    Tree::Node const& parent = this->levels[lvl_idx - 1][node.parent_idx];
                    stream << "\t" << node.id << "[label=\"" << node.id << "(w:" << node.weight << ")\"]\n";
                    stream << "\t" << parent.id;
                    stream << " -> " << node.id;
                    stream << "[label=\"" << node.parent_edge_weight << "\"";
                    stream << "]\n";
                }
            }

            int32_t invis_node = -1;
            for (size_t lvl_idx = this->levels.size() - 1; lvl_idx > 0; --lvl_idx) {
                std::vector<Tree::Node> const& lvl = this->levels[lvl_idx];
                std::stringstream node_ordering; 
                node_ordering << "{rank=same " << lvl[0].id;
                for (size_t node_idx = 1; node_idx < this->levels[lvl_idx].size(); ++node_idx) {
                    Tree::Node const& node = lvl[node_idx];
                    stream << "\t" << invis_node << "[label=\"\", width=0.1, style=invis]\n";
                    stream << "\t" << this->levels[lvl_idx - 1][node.parent_idx].id;
                    stream << " -> " << invis_node << "[style=invis]\n";
                    node_ordering << " -> " << invis_node << " -> " << node.id;
                    --invis_node;
                }
                if (lvl.size() > 1) {
                    node_ordering << "[style=invis]";
                }
                node_ordering << "}\n";
                stream << node_ordering.str();
            }

            stream << "}\n";
            return stream.str();
        }

    template<typename Id, typename EdgeWeight>
        std::istream& operator>>(std::istream& is, Tree<Id, int, EdgeWeight>& tree) {
            Id node_cnt;
            Id root_id;
            is >> node_cnt >> root_id;

            std::map<Id, std::map<Id, EdgeWeight>> tree_map;
            for (Id edge_idx = 0; edge_idx < node_cnt - 1; ++edge_idx) {
                Id from; 
                Id to; 
                EdgeWeight weight;
                is >> from >> to >> weight;
                tree_map[from][to] = weight;
                tree_map[to][from] = weight;
            }

            std::map<Id, int> node_weight;
            for (auto const& node : tree_map) {
                node_weight[node.first] = 1;
            }

            tree = Tree<Id, EdgeWeight>::build_tree(tree_map, node_weight, root_id);

            return is;
        }

    template<typename Id, typename EdgeWeight>
        std::ostream& operator<<(std::ostream& os, Tree<Id, int, EdgeWeight> const& tree) {
            size_t node_cnt = 0;
            for (auto const& lvl: tree.levels) {
                node_cnt += lvl.size();
            }

            os << node_cnt << " " << tree.levels[0][0].id << std::endl;
            for (size_t lvl_idx = 1; lvl_idx < tree.levels.size(); ++lvl_idx) {
                for (auto const& node : tree.levels[lvl_idx]) {
                    os << tree.levels[lvl_idx - 1][node.parent_idx].id << " ";
                    os << node.id << " " << node.parent_edge_weight << std::endl;
                }
            }
            return os;
        }

    template<typename Id, typename NodeWeight, typename EdgeWeight>
        std::ostream& operator<<(
                std::ostream& os, 
                SignaturesForTree<Id, NodeWeight, EdgeWeight> const& signatures) 
        {
            os << signatures.part_cnt << " ";  
            os << signatures.eps.get_num() << " " << signatures.eps.get_den() << std::endl;
            os << std::endl;

            auto const& sigs = signatures.signatures;
            for (size_t lvl_idx = 0; lvl_idx < sigs.size(); ++lvl_idx) {
                for (size_t node_idx = 0; node_idx < sigs[lvl_idx].size(); ++node_idx) {
                    auto const& node_sigs = sigs[lvl_idx][node_idx];
                    os << signatures.tree.levels[lvl_idx][node_idx].id << " ";
                    os << node_sigs.size() << std::endl;

                    Id node_sigs_weight = 0;
                    for (auto const& node_sigs_with_weight : node_sigs) {
                        os << node_sigs_weight << " ";
                        os << node_sigs_with_weight.size() << std::endl;
                        for (auto const& sig : node_sigs_with_weight) {
                            for (auto const& val : sig.first) {
                                os << val << " ";
                            }
                            os << sig.second << std::endl;
                        }
                        node_sigs_weight += 1;
                    }
                    os << std::endl;
                }
            }
            return os;
        }

    template<typename Id, typename NodeWeight, typename EdgeWeight>
        SignaturesForTreeBuilder<Id, NodeWeight, EdgeWeight>& 
            SignaturesForTreeBuilder<Id, NodeWeight, EdgeWeight>::with_part_cnt(NodeWeight part_cnt) {
            this->part_cnt = part_cnt;
            return *this;
        }

    template<typename Id, typename NodeWeight, typename EdgeWeight>
        SignaturesForTreeBuilder<Id, NodeWeight, EdgeWeight>& 
            SignaturesForTreeBuilder<Id, NodeWeight, EdgeWeight>::with_eps(Rational eps) {

        this->eps = eps;
        return *this;
    }

    template<typename Id, typename NodeWeight, typename EdgeWeight>
    SignaturesForTreeBuilder<Id, NodeWeight, EdgeWeight>& 
    SignaturesForTreeBuilder<Id, NodeWeight, EdgeWeight>::with_signatures(
            std::vector<std::vector<SignatureMap>> const& signatures) {
        this->signatures = signatures;
        return *this;
    }

    template<typename Id, typename NodeWeight, typename EdgeWeight>
        SignaturesForTree<Id, NodeWeight, EdgeWeight>
        SignaturesForTreeBuilder<Id, NodeWeight, EdgeWeight>::finish() {
            return SignaturesForTree<Id, NodeWeight, EdgeWeight>(
                    this->part_cnt, this->eps, this->tree, this->signatures);
        }

    template<typename Id, typename NodeWeight, typename EdgeWeight>
        std::istream& operator>>(
                std::istream& is, 
                SignaturesForTreeBuilder<Id, NodeWeight, EdgeWeight>& builder)
        {
            NodeWeight part_cnt;
            int64_t eps_num;
            int64_t eps_denom;
            is >> part_cnt >> eps_num >> eps_denom;
            Rational eps(eps_num, eps_denom);
            builder.with_part_cnt(part_cnt).with_eps(eps);
            auto signature_length = calculate_upper_component_weight_bounds(
                    eps, builder.tree.subtree_weight[0][0], part_cnt).size();

            std::vector<std::vector<SignatureMap<NodeWeight, EdgeWeight>>> signatures;

            size_t node_cnt = 0;
            for (auto const& lvl : builder.tree.levels) {
                signatures.emplace_back(lvl.size());
                node_cnt += lvl.size();
            }

            for (Id node_idx = 0; static_cast<size_t>(node_idx) < node_cnt; ++node_idx) {
                Id node_id;
                NodeWeight weight_cnt;
                is >> node_id >> weight_cnt;
                auto node_idx_in_tree = builder.tree.get_node_idx(node_id);

                NodeWeight max_sig_weight = 1;
                for (size_t sibling_idx = 0; sibling_idx <= node_idx_in_tree.second; ++sibling_idx) {
                    max_sig_weight += builder.tree.subtree_weight[node_idx_in_tree.first][sibling_idx];
                }

                SignatureMap<NodeWeight, EdgeWeight>& node_sigs = signatures[node_idx_in_tree.first][node_idx_in_tree.second];
                node_sigs = SignatureMap<NodeWeight, EdgeWeight>(static_cast<size_t>(max_sig_weight));

                for (NodeWeight weight_idx = 0; weight_idx < weight_cnt; ++weight_idx) {
                    NodeWeight weight;
                    size_t signature_cnt;
                    is >> weight >> signature_cnt;

                    for (size_t signature_idx = 0; signature_idx < signature_cnt; ++signature_idx) {
                        std::valarray<NodeWeight> signature(signature_length);
                        EdgeWeight cut_cost;
                        for (auto& comp : signature) {
                            is >> comp;
                        }
                        is >> cut_cost;
                        node_sigs.at(weight)[signature] = cut_cost;
                    }
                }
            }
            builder.with_signatures(signatures);

            return is;
        }
}
