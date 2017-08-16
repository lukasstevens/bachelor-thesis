namespace cut {

    template<typename IdType, typename EdgeWeightType>
        std::pair<size_t, size_t> Tree<IdType, EdgeWeightType>::get_node_idx(IdType node_id) const {
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

    template<typename IdType, typename EdgeWeightType>
        std::string Tree<IdType, EdgeWeightType>::as_graphviz() const {

            std::stringstream stream;
            stream << "digraph tree {\n";
            for (size_t lvl_idx = this->levels.size() - 1; lvl_idx > 0; --lvl_idx) {
                for (size_t node_idx = 0; node_idx < this->levels[lvl_idx].size(); ++node_idx) {
                    Tree::Node const& node = this->levels[lvl_idx][node_idx];
                    Tree::Node const& parent = this->levels[lvl_idx - 1][node.parent_idx];
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

    template<typename IdType, typename EdgeWeightType>
        std::istream& operator>>(std::istream& is, Tree<IdType, EdgeWeightType>& tree) {
            using SizeType = IdType;
            SizeType node_cnt;
            IdType root_id;
            is >> node_cnt >> root_id;

            std::map<IdType, std::map<IdType, EdgeWeightType>> tree_map;
            for (SizeType edge_idx = 0; edge_idx < node_cnt - 1; ++edge_idx) {
                IdType from; 
                IdType to; 
                EdgeWeightType weight;
                is >> from >> to >> weight;
                tree_map[from][to] = weight;
            }
            tree = Tree<IdType, EdgeWeightType>::build_tree(tree_map, root_id);

            return is;
        }

    template<typename IdType, typename EdgeWeightType>
        std::ostream& operator<<(std::ostream& os, Tree<IdType, EdgeWeightType> const& tree) {
            os << tree.tree_sizes[0][0] << " " << tree.levels[0][0].id << std::endl;
            for (size_t lvl_idx = 1; lvl_idx < tree.levels.size(); ++lvl_idx) {
                for (auto const& node : tree.levels[lvl_idx]) {
                    os << tree.levels[lvl_idx - 1][node.parent_idx].id << " ";
                    os << node.id << " " << node.parent_edge_weight << std::endl;
                }
            }
            return os;
        }

    template<typename IdType, typename EdgeWeightType>
        std::ostream& operator<<(std::ostream& os, SignaturesForTree<IdType, EdgeWeightType> const& signatures) {
            os << signatures.part_cnt << " ";  
            os << signatures.eps.get_num() << " " << signatures.eps.get_den() << std::endl;
            os << std::endl;

            auto const& sigs = signatures.signatures;
            for (size_t lvl_idx = 0; lvl_idx < sigs.size(); ++lvl_idx) {
                for (size_t node_idx = 0; node_idx < sigs[lvl_idx].size(); ++node_idx) {
                    auto const& node_sigs = sigs[lvl_idx][node_idx];
                    os << signatures.tree.levels[lvl_idx][node_idx].id << " ";
                    os << node_sigs.size() << std::endl;

                    IdType node_sigs_size = 0;
                    for (auto const& node_sigs_with_size : node_sigs) {
                        os << node_sigs_size << " ";
                        os << node_sigs_with_size.size() << std::endl;
                        for (auto const& sig : node_sigs_with_size) {
                            for (auto const& val : sig.first) {
                                os << val << " ";
                            }
                            os << sig.second << std::endl;
                        }
                        node_sigs_size += 1;
                    }
                    os << std::endl;
                }
            }
            return os;
        }

    template<typename IdType, typename EdgeWeightType>
        SignaturesForTreeBuilder<IdType, EdgeWeightType>& 
            SignaturesForTreeBuilder<IdType, EdgeWeightType>::with_part_cnt(IdType part_cnt) {

            this->part_cnt = part_cnt;
            return *this;
        }

    template<typename IdType, typename EdgeWeightType>
        SignaturesForTreeBuilder<IdType, EdgeWeightType>& 
            SignaturesForTreeBuilder<IdType, EdgeWeightType>::with_eps(RationalType eps) {

        this->eps = eps;
        return *this;
    }

    template<typename IdType, typename EdgeWeightType>
    SignaturesForTreeBuilder<IdType, EdgeWeightType>& SignaturesForTreeBuilder<IdType, EdgeWeightType>::with_signatures(
            std::vector<std::vector<SignatureMap>> const& signatures) {
        this->signatures = signatures;
        return *this;
    }

    template<typename IdType, typename EdgeWeightType>
        SignaturesForTree<IdType, EdgeWeightType> SignaturesForTreeBuilder<IdType, EdgeWeightType>::finish() {
            return SignaturesForTree<IdType, EdgeWeightType>(this->part_cnt, this->eps, this->tree, this->signatures);
        }

    template<typename IdType, typename EdgeWeightType>
        std::istream& operator>>(std::istream& is, SignaturesForTreeBuilder<IdType, EdgeWeightType>& builder) {
            using SizeType = IdType;

            SizeType part_cnt;
            int64_t eps_num;
            int64_t eps_denom;
            is >> part_cnt >> eps_num >> eps_denom;
            RationalType eps(eps_num, eps_denom);
            builder.with_part_cnt(part_cnt).with_eps(eps);
            auto signature_length = calculate_upper_component_size_bounds(
                    eps, builder.tree.tree_sizes[0][0], part_cnt).size();

            std::vector<std::vector<SignatureMap<IdType, EdgeWeightType>>> signatures;
            for (auto const& level : builder.tree.levels) {
                signatures.emplace_back(level.size());
            }

            for (SizeType node_idx = 0; node_idx < builder.tree.tree_sizes[0][0]; ++node_idx) {
                IdType node_id;
                SizeType size_cnt;
                is >> node_id >> size_cnt;
                auto node_idx_in_tree = builder.tree.get_node_idx(node_id);

                SizeType max_sig_size = 1;
                for (size_t sibling_idx = 0; sibling_idx <= node_idx_in_tree.second; ++sibling_idx) {
                    max_sig_size += builder.tree.tree_sizes[node_idx_in_tree.first][sibling_idx];
                }

                SignatureMap<IdType, EdgeWeightType>& node_sigs = signatures[node_idx_in_tree.first][node_idx_in_tree.second];
                node_sigs = SignatureMap<IdType, EdgeWeightType>(static_cast<size_t>(max_sig_size));

                for (SizeType size_idx = 0; size_idx < size_cnt; ++size_idx) {
                    SizeType size;
                    SizeType signature_cnt;
                    is >> size >> signature_cnt;

                    for (SizeType signature_idx = 0; signature_idx < signature_cnt; ++signature_idx) {
                        std::valarray<SizeType> signature(signature_length);
                        EdgeWeightType cut_cost;
                        for (auto& comp : signature) {
                            is >> comp;
                        }
                        is >> cut_cost;
                        node_sigs[static_cast<size_t>(size)][signature] = cut_cost;
                    }
                }
            }
            builder.with_signatures(signatures);

            return is;
        }
}