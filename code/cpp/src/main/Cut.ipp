namespace cut {

    template<typename IdType, typename EdgeWeightType>
        Node<IdType, EdgeWeightType>::Node(IdType id, EdgeWeightType parent_edge_weight, 
                size_t parent_idx, std::pair<size_t, size_t> children_idx_range) : 
            id(id), parent_edge_weight(parent_edge_weight), parent_idx(parent_idx), children_idx_range(children_idx_range) {}


    template<typename IdType, typename EdgeWeightType>
        Tree<IdType, EdgeWeightType> Tree<IdType, EdgeWeightType>::build_tree(std::map<IdType, std::map<IdType, EdgeWeightType>> const& tree_map, IdType root_id) {
            Tree tree;
            // Use a struct to represent an incomplete node since the child_idx_range is not known.
            struct NodeStub {
                IdType const id; 
                EdgeWeightType const parent_edge_weight;
                size_t const parent_idx;
                bool const has_left_sibling;
                size_t const level;

                NodeStub(IdType id, EdgeWeightType p_e_w, size_t p_i, bool h_l_s, size_t lvl) :
                    id(id), parent_edge_weight(p_e_w), parent_idx(p_i), has_left_sibling(h_l_s), level(lvl) {}
            };

            // Do a BFS to build the tree.
            std::list<NodeStub> queue;
            queue.push_back(NodeStub(root_id, 0, 0, false, 0));
            size_t next_child_idx = 0;
            while(!queue.empty()) {
                NodeStub curr_node = queue.front();
                queue.pop_front();

                // Add a new level to tree if the BFS reaches a new level and reset the child_idx.
                if (curr_node.level == tree.levels.size()) {
                    tree.levels.push_back(std::vector<Node>());
                    tree.has_left_sibling.push_back(std::vector<bool>());
                    next_child_idx = 0;
                }

                // Save the index of the first child.
                size_t old_next_child_idx = next_child_idx;
                if (tree_map.find(curr_node.id) != tree_map.cend()) {
                    for (auto const neighbor : tree_map.at(curr_node.id)) {
                        // Check if neighbor is the parent.
                        if (curr_node.level == 0 || neighbor.first != tree.levels[curr_node.level - 1][curr_node.parent_idx].id) {
                            bool curr_has_left_sibling = !(old_next_child_idx == next_child_idx);
                            queue.emplace_back(neighbor.first, neighbor.second, tree.levels[curr_node.level].size(), curr_has_left_sibling, curr_node.level + 1); 
                            ++next_child_idx;
                        }
                    }
                }

                tree.levels[curr_node.level].emplace_back(curr_node.id, curr_node.parent_edge_weight, 
                        curr_node.parent_idx, std::make_pair(old_next_child_idx, next_child_idx));
                tree.has_left_sibling[curr_node.level].push_back(curr_node.has_left_sibling);

            }

            tree.calculate_subtree_sizes();

            return tree;
        }

    template<typename IdType, typename EdgeWeightType>
        void Tree<IdType, EdgeWeightType>::calculate_subtree_sizes() {
            Tree& tree = *this;
            for (auto const& lvl : tree.levels) {
                tree.tree_sizes.emplace_back(lvl.size(), 1);
            }

            for (size_t lvl_idx = tree.levels.size() - 2; lvl_idx < tree.levels.size(); --lvl_idx) {
                size_t node_idx = 0;
                for (auto const& node : tree.levels[lvl_idx]) {
                    for (size_t child_idx = node.children_idx_range.first; 
                            child_idx < node.children_idx_range.second; ++child_idx) {
                        tree.tree_sizes[lvl_idx][node_idx] += tree.tree_sizes[lvl_idx + 1][child_idx];
                    }
                    ++node_idx;
                }
            }
        }

    template<typename IdType, typename EdgeWeightType>
        Tree<IdType, EdgeWeightType> Tree<IdType, EdgeWeightType>::build_tree(
                std::map<IdType, std::map<IdType, EdgeWeightType>> const& tree_map) {
            return build_tree(tree_map, tree_map.begin()->first);
        }

    template<typename IdType, typename EdgeWeightType>
        SignatureMap<IdType, EdgeWeightType> Tree<IdType, EdgeWeightType>::cut_at_node(
                Tree::Node const& node, 
                Tree::SizeType node_subtree_size,
                Tree::SizeType left_siblings_size,
                Tree::SignatureMap const& left_sibling_sigs, 
                Tree::SignatureMap const& right_child_sigs, 
                std::vector<Tree::SizeType> const& comp_size_bounds) {

            // The maximum amount of nodes in a Signature at the current node is the frontier size
            // which is left_sibling_size + node_subtree_size plus 1
            Tree::SignatureMap node_sigs(static_cast<size_t>(left_siblings_size + node_subtree_size) + 1);

            // Iterate over all calculated signatures of the left sibling and the rightmost child according
            // to the dynamic programming scheme described in the paper FF13.
            for (Tree::SizeType left_sibling_node_cnt = 0; 
                    static_cast<size_t>(left_sibling_node_cnt) < left_sibling_sigs.size(); 
                    ++left_sibling_node_cnt) {

                auto const& left_sibling_sigs_with_node_cnt = left_sibling_sigs[static_cast<size_t>(left_sibling_node_cnt)];

                for (Tree::SizeType child_node_cnt = 0; 
                        static_cast<size_t>(child_node_cnt) < right_child_sigs.size(); ++child_node_cnt) {

                    auto const& child_sigs_with_node_cnt = right_child_sigs[static_cast<size_t>(child_node_cnt)];

                    for (auto const& left_sibling_sig : left_sibling_sigs_with_node_cnt) {
                        for (auto const& child_sig : child_sigs_with_node_cnt) {
                            // First case: The edge from the current node to its parent is not cut.
                            Tree::SizeType frontier_size = left_sibling_node_cnt + child_node_cnt;
                            size_t frontier_size_size_t = static_cast<size_t>(frontier_size);
                            EdgeWeightType cut_cost = left_sibling_sig.second + child_sig.second;
                            Tree::Signature sig = left_sibling_sig.first + child_sig.first;

                            auto prev_cut_cost_it = node_sigs[frontier_size_size_t].find(sig);
                            if (prev_cut_cost_it == node_sigs[frontier_size_size_t].end()
                                    || cut_cost < prev_cut_cost_it->second) {
                                node_sigs[frontier_size_size_t][sig] = cut_cost;
                            }

                            // Second case: The edge from the current node to its parent is cut.
                            Tree::SizeType const node_comp_size = node_subtree_size - child_node_cnt;
                            // Check if the current size of the component which includes the current node is smaller than
                            // the maximum allowed size.
                            if (node_comp_size >= comp_size_bounds.back()) {
                                continue;
                            } else {
                                frontier_size += node_comp_size;
                                frontier_size_size_t = static_cast<size_t>(frontier_size);
                                cut_cost += node.parent_edge_weight;

                                // Adjust the signature to account for the component which contains the current node.
                                size_t i = 0; 
                                while (node_comp_size >= comp_size_bounds[i]) { ++i; }
                                sig[i] += 1;

                                prev_cut_cost_it = node_sigs[frontier_size_size_t].find(sig);
                                if (prev_cut_cost_it == node_sigs[frontier_size_size_t].end()
                                        || cut_cost < prev_cut_cost_it->second) {
                                    node_sigs[frontier_size_size_t][sig] = cut_cost;
                                }
                            }
                        }
                    }
                }
            }
            return node_sigs;
        }


    template<typename IdType, typename EdgeWeightType>
        SignaturesForTree<IdType, EdgeWeightType> Tree<IdType, EdgeWeightType>::cut(RationalType eps, SizeType part_cnt) {

            std::vector<std::vector<Tree::SignatureMap>> signatures;
            for (auto const& lvl : this->levels) {
                signatures.emplace_back(lvl.size());
            }

            // Calculate the size intervals of the connected components of a signature.
            std::vector<SizeType> const comp_size_bounds 
                = calculate_upper_component_size_bounds(eps, this->tree_sizes[0][0], part_cnt);

            // Iterate over all nodes except the root starting with the node one the bottom left.
            for (size_t lvl_idx = this->levels.size() - 1; lvl_idx > 0; --lvl_idx) {
                Tree::SizeType left_siblings_size = 0; 
                for (size_t node_idx = 0; node_idx < this->levels[lvl_idx].size(); ++node_idx) {
                    Tree::Node const& node = this->levels[lvl_idx][node_idx];
                    Tree::SizeType const node_subtree_size = this->tree_sizes[lvl_idx][node_idx];
                    bool const node_has_left_sibling = this->has_left_sibling[lvl_idx][node_idx];
                    bool const node_has_child = node.children_idx_range.first < node.children_idx_range.second;

                    // The signature which contains 0 nodes, is the 0-vector and has 0 cut cost is
                    // always present, even if the node does not exist.
                    Tree::SignatureMap const empty_map({{{{Signature(comp_size_bounds.size()), 0}}}});

                    Tree::SignatureMap const* left_sibling_sigs = &empty_map;
                    Tree::SignatureMap const* child_sigs = &empty_map;

                    // Adjust the reference to the signatures if the node has a left sibling or
                    // has a child respectively.
                    // TODO: Think about optimizing the case where there is no left sibling or there is no child.
                    // At the moment we are adding the 0-vector to all signatures calculated beforehand which is 
                    // unnecessary.
                    if (node_has_left_sibling) {
                        left_sibling_sigs = &signatures[lvl_idx][node_idx - 1];
                    }
                    if (node_has_child) {
                        child_sigs = &signatures[lvl_idx + 1][node.children_idx_range.second - 1];
                    }


                    signatures[lvl_idx][node_idx] = cut_at_node(node, node_subtree_size, left_siblings_size,
                            *left_sibling_sigs, *child_sigs, comp_size_bounds);
                    left_siblings_size += node_subtree_size;
                }
            }

            // Calculate the signatures at the root according to the paper FF13. 
            // Signatures which contain less then the total amount of nodes are ignored.
            Tree::SizeType const node_cnt = this->tree_sizes[0][0];
            size_t const node_cnt_size_t = static_cast<size_t>(node_cnt);
            Tree::SignatureMap& root_sigs = signatures[0][0];
            root_sigs = SignatureMap(node_cnt_size_t + 1);
            Tree::SignatureMap& child_sigs = signatures[1].back();

            for (Tree::SizeType root_comp_node_cnt = comp_size_bounds.back() - 1; 
                    root_comp_node_cnt > 0; --root_comp_node_cnt) {
                for (auto const& sig : child_sigs[static_cast<size_t>(node_cnt - root_comp_node_cnt)]) {
                    Tree::Signature root_sig(sig.first);
                    size_t i = 0;
                    while(root_comp_node_cnt >= comp_size_bounds[i]) { ++i; }
                    root_sig[i] += 1;

                    auto prev_cut_cost_it = root_sigs[node_cnt_size_t].find(root_sig);
                    if (prev_cut_cost_it == root_sigs[node_cnt_size_t].end()
                            || sig.second < prev_cut_cost_it->second) {
                        root_sigs[node_cnt_size_t][root_sig] = sig.second;
                    }
                }
            }

            return SignaturesForTree<IdType, EdgeWeightType>(part_cnt, eps, *this, signatures);
        }

    template<typename IdType, typename EdgeWeightType>
       SignatureMapWithPrev<IdType, EdgeWeightType> Tree<IdType, EdgeWeightType>::cut_at_node_with_prev(
                Tree::Node const& node, 
                Tree::SizeType subtree_size,
                Tree::SignatureMapWithPrev const& left_sibling_sigs, 
                Tree::SignatureMapWithPrev const& right_child_sigs,
                std::vector<Tree::SizeType> const& upper_comp_size_bounds,
                Tree::Signature const& signature
                ) {

            // Iterate over all calculated signatures of the left sibling and the rightmost child according
            // to the dynamic programming scheme described in the paper FF13.
            Tree::SignatureMapWithPrev node_sigs;
            for (auto const& left_sibling_sigs_with_size : left_sibling_sigs) {
                for (auto const& child_sigs_with_size : right_child_sigs) {
                    for (auto const& left_sibling_sig : left_sibling_sigs_with_size.second) {
                        for (auto const& child_sig : child_sigs_with_size.second) {
                            // First case: The edge from the current node to its parent is not cut.
                            Tree::SizeType frontier_size = left_sibling_sigs_with_size.first + child_sigs_with_size.first;

                            // The entries in SignatureMapWithPrev are pairs of the cut cost and the previous signatures. 
                            EdgeWeightType left_sibling_sig_cut_cost = left_sibling_sig.second.first;
                            EdgeWeightType child_sig_cut_cost = child_sig.second.first;

                            EdgeWeightType cut_cost = left_sibling_sig_cut_cost + child_sig_cut_cost;
                            Tree::Signature node_sig = left_sibling_sig.first + child_sig.first;

                            // Check if the current signature is feasible.
                            if (!((node_sig <= signature).min())) {
                                continue;
                            }

                            PreviousSignatures<IdType> previous_signatures(
                                    std::make_pair(left_sibling_sigs_with_size.first, left_sibling_sig.first),
                                    std::make_pair(child_sigs_with_size.first, child_sig.first),
                                    false);

                            if (node_sigs[frontier_size].find(node_sig) == node_sigs[frontier_size].end()
                                    || cut_cost < node_sigs[frontier_size][node_sig].first) {
                                node_sigs[frontier_size][node_sig] = std::make_pair(cut_cost, previous_signatures);
                            } 

                            // Second case: The edge from the current node to its parent is cut.
                            // Check if the current size of the component which includes the current node is smaller than
                            // the maximum allowed size.
                            Tree::SizeType const node_comp_size = subtree_size - child_sigs_with_size.first;
                            if (node_comp_size >= upper_comp_size_bounds.back()) {
                                continue;
                            } else {
                                frontier_size += node_comp_size;
                                cut_cost += node.parent_edge_weight;

                                // Adjust the signature to account for the component which contains the current node.
                                size_t i = 0; 
                                while (node_comp_size >= upper_comp_size_bounds[i]) { ++i; }
                                node_sig[i] += 1;
                                // Check if the current signature is feasible.
                                if (!((node_sig <= signature).min())) {
                                    continue;
                                }

                                previous_signatures.was_parent_edge_cut = true;
                                if (node_sigs[frontier_size].find(node_sig) == node_sigs[frontier_size].end()
                                        || cut_cost < node_sigs[frontier_size][node_sig].first) {
                                    node_sigs[frontier_size][node_sig] = std::make_pair(cut_cost, previous_signatures);
                                } 
                            }
                        }
                    }
                }
            }
            return node_sigs;
        }

    template<typename IdType, typename EdgeWeightType>
        std::vector<std::vector<SignatureMapWithPrev<IdType, EdgeWeightType>>> 
        Tree<IdType, EdgeWeightType>::cut_with_prev(RationalType eps, SizeType part_cnt, Tree::Signature const& signature) const {

            auto const upper_comp_size_bounds = calculate_upper_component_size_bounds(eps, this->tree_sizes[0][0], part_cnt);
            std::vector<std::vector<Tree::SignatureMapWithPrev>> signatures_with_prev;
            for (auto const& lvl : this->levels) {
                signatures_with_prev.emplace_back(lvl.size());
            }

            // Iterate over all nodes except the root starting with the node one the bottom left.
            for (size_t lvl_idx = this->levels.size() - 1; lvl_idx > 0; --lvl_idx) {
                for (size_t node_idx = 0; node_idx < this->levels[lvl_idx].size(); ++node_idx) {
                    Tree::Node const& node = this->levels[lvl_idx][node_idx];
                    Tree::SizeType const node_subtree_size = this->tree_sizes[lvl_idx][node_idx];
                    Tree::SignatureMapWithPrev empty_map;
                    // The only signature which always has cut value smaller infinity(even if the node does not exist) is the 0-vector.
                    Tree::Signature const empty_signature = Tree::Signature(signature.size());
                    std::pair<Tree::SizeType, Tree::Signature> empty_prev_signature(0, empty_signature);
                    empty_map[0][empty_signature] = std::make_pair(0, 
                            PreviousSignatures<IdType>(empty_prev_signature, empty_prev_signature, false));

                    Tree::SignatureMapWithPrev const* left_sibling_sigs = &empty_map;
                    Tree::SignatureMapWithPrev const* child_sigs = &empty_map;

                    // Adjust the reference to the signatures if the node has a left sibling or
                    // has a child respectively.
                    bool const node_has_left_sibling = this->has_left_sibling[lvl_idx][node_idx];
                    bool const node_has_child = node.children_idx_range.first < node.children_idx_range.second;
                    if (node_has_left_sibling) {
                        left_sibling_sigs = &signatures_with_prev[lvl_idx][node_idx - 1];
                    }
                    if (node_has_child) {
                        child_sigs = &signatures_with_prev[lvl_idx + 1][node.children_idx_range.second - 1];
                    }

                    signatures_with_prev[lvl_idx][node_idx] = 
                        cut_at_node_with_prev(node, node_subtree_size, *left_sibling_sigs, *child_sigs, 
                                upper_comp_size_bounds, signature);

                }

                // Calculate the signatures at the root according to the paper FF13. 
                // Signatures which contain less then the total amount of nodes are ignored.
                Tree::SignatureMapWithPrev& root_sigs = signatures_with_prev[0][0];
                Tree::SizeType const node_cnt = this->tree_sizes[0][0];
                for (auto const& child_sigs_with_size : signatures_with_prev[1].back()) {
                    Tree::SizeType const root_comp_size = node_cnt - child_sigs_with_size.first;
                    if (root_comp_size >= upper_comp_size_bounds.back()) {
                        continue;
                    } else {
                        for (auto const& child_sig : child_sigs_with_size.second) {

                            // Initialize root_sig equal to child_sig and then adjust for the size
                            // of the component which contians the root.
                            Tree::Signature root_sig(child_sig.first);
                            size_t i = 0;
                            while(root_comp_size >= upper_comp_size_bounds[i]) { ++i; }
                            root_sig[i] += 1;

                            EdgeWeightType cut_cost = child_sig.second.first;

                            PreviousSignatures<IdType> previous_signatures(
                                    std::make_pair(0, Signature(signature.size())),
                                    std::make_pair(child_sigs_with_size.first, child_sig.first),
                                    false);

                            if (root_sigs[node_cnt].find(root_sig) == root_sigs[node_cnt].end() || 
                                    cut_cost < root_sigs[node_cnt][root_sig].first) {
                                root_sigs[node_cnt][root_sig] = std::make_pair(cut_cost, previous_signatures);
                            }
                        }
                    }
                }
            }

            return signatures_with_prev;
        }

    template<typename SizeType>
        std::vector<SizeType> calculate_upper_component_size_bounds(RationalType eps, SizeType node_cnt, SizeType part_cnt) {
            using Rational = RationalType;

            // Calculate the sizes of the components in a signature according to the paper FF13.
            // We use rationals here to prevent numerical instabilities.
            std::vector<SizeType> comp_sizes;
            Rational n_div_k = Rational(gmputils::ceil_to_int<SizeType>(Rational(node_cnt, part_cnt)));
            Rational curr_upper_bound = eps * n_div_k;
            Rational upper_bound = (Rational(1) + eps) * n_div_k;
            while (curr_upper_bound < upper_bound) {
                comp_sizes.push_back(gmputils::ceil_to_int<SizeType>(curr_upper_bound));
                curr_upper_bound *= (Rational(1) + eps);
            }
            comp_sizes.push_back(gmputils::floor_to_int<SizeType>(upper_bound + Rational(1)));
            return comp_sizes;
        }

    template<typename SizeType>
        std::vector<SizeType> calculate_lower_component_size_bounds(RationalType eps, SizeType node_cnt, SizeType part_cnt) {

            std::vector<SizeType> const upper_comp_size_bounds 
                = calculate_upper_component_size_bounds(eps, node_cnt, part_cnt);
            std::vector<SizeType> lower_comp_size_bounds({1});
            lower_comp_size_bounds.insert(
                    lower_comp_size_bounds.end(), 
                    upper_comp_size_bounds.begin(), 
                    upper_comp_size_bounds.end() - 1);

            return lower_comp_size_bounds;
        }

    template<typename IdType, typename EdgeWeightType>
        typename SignaturesForTree<IdType, EdgeWeightType>::CutEdges 
        SignaturesForTree<IdType, EdgeWeightType>::cut_edges_for_signature(
                SignaturesForTree::Signature const& signature) const {

            std::vector<std::vector<SignatureMapWithPrev<IdType, EdgeWeightType>>> signatures_with_prev 
                = this->tree.cut_with_prev(this->eps, this->part_cnt, signature);
            SignaturesForTree::CutEdges cut_edges;

            struct SignatureAtNode {
                std::pair<SignaturesForTree::SizeType, SignaturesForTree::Signature> sig_with_size;
                std::pair<size_t, size_t> node_idx;

                SignatureAtNode(
                        std::pair<SizeType, Signature> sig_with_size, 
                        std::pair<size_t, size_t> node_idx) :
                    sig_with_size(sig_with_size), node_idx(node_idx) {}
            };

            std::list<SignatureAtNode> queue; 
            queue.emplace_back(std::make_pair(this->tree.tree_sizes[0][0], signature), std::make_pair(0, 0));
            while (!queue.empty()) {
                SignatureAtNode const sig_at_node = queue.front();
                queue.pop_front();

                Node<IdType, EdgeWeightType> const& node 
                    = this->tree.levels[sig_at_node.node_idx.first][sig_at_node.node_idx.second];
                auto const& node_idx = sig_at_node.node_idx;

                SignatureMapWithPrev<IdType, EdgeWeightType> const& signatures_with_prev_at_node = 
                    signatures_with_prev[node_idx.first][node_idx.second];
                PreviousSignatures<IdType> const& previous_signatures = signatures_with_prev_at_node
                    .at(sig_at_node.sig_with_size.first).at(sig_at_node.sig_with_size.second).second;

                bool const node_has_left_sibling = this->tree.has_left_sibling[node_idx.first][node_idx.second];
                bool const node_has_child = node.children_idx_range.first < node.children_idx_range.second;
                if (node_has_left_sibling) {
                    auto left_sibling_idx(sig_at_node.node_idx);
                    left_sibling_idx.second -= 1;
                    queue.emplace_back(
                            previous_signatures.left_sibling_sig,
                            left_sibling_idx);
                }

                if (node_has_child) {
                    auto right_child_idx(sig_at_node.node_idx);
                    right_child_idx.first += 1;
                    right_child_idx.second = node.children_idx_range.second - 1;
                    queue.emplace_back(
                            previous_signatures.right_child_sig,
                            right_child_idx);
                }

                if (previous_signatures.was_parent_edge_cut) {
                    Node<IdType, EdgeWeightType> const& parent = this->tree.levels[sig_at_node.node_idx.first - 1][node.parent_idx];
                    cut_edges.emplace(node.id, parent.id);
                }
            }

            return cut_edges;
        }

    template<typename IdType, typename EdgeWeightType>
        std::vector<std::set<IdType>> SignaturesForTree<IdType, EdgeWeightType>::components_for_cut_edges(
            SignaturesForTree::CutEdges const& cut_edges) const {

            std::vector<std::set<IdType>> components;

            struct NodeInfo {
                std::pair<size_t, size_t> node_idx;
                size_t component_idx;

                NodeInfo(std::pair<size_t, size_t> node_idx, size_t component_idx) :
                    node_idx(node_idx), component_idx(component_idx) {}
            };

            components.emplace_back();
            std::list<NodeInfo> queue;

            queue.emplace_back(std::make_pair(0, 0), 0);

            while (!queue.empty()) {
                auto curr_info = queue.front();
                Node<IdType, EdgeWeightType> const& curr_node = this->tree.levels[curr_info.node_idx.first][curr_info.node_idx.second];
                queue.pop_front();

                components[curr_info.component_idx].insert(curr_node.id);
                for (size_t child_idx = curr_node.children_idx_range.first; 
                        child_idx < curr_node.children_idx_range.second; ++child_idx) {
                    Node<IdType, EdgeWeightType> const& child = this->tree.levels[curr_info.node_idx.first + 1][child_idx];
                    std::pair<size_t, size_t> const child_node_idx(curr_info.node_idx.first + 1, child_idx);
                    if (cut_edges.find(std::make_pair(child.id, curr_node.id)) != cut_edges.end() || 
                            cut_edges.find(std::make_pair(curr_node.id, child.id)) != cut_edges.end()) {
                        components.emplace_back();
                        queue.emplace_back(child_node_idx, components.size() - 1);
                    } else {
                        queue.emplace_back(child_node_idx, curr_info.component_idx);
                    }
                }
            }

            return components;
        }

}
