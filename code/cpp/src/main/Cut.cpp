#include<cmath>
#include<limits>
#include<list>
#include<stdexcept>

#include "GMPUtils.hpp"

#include "Cut.hpp"

namespace cut {

    using EdgeWeightType = Node::EdgeWeightType;
    using IdType = Node::IdType;

    Node::Node(IdType id, EdgeWeightType parent_edge_weight, 
            size_t parent_idx, std::pair<size_t, size_t> children_idx_range) : 
        id(id), parent_edge_weight(parent_edge_weight), parent_idx(parent_idx), children_idx_range(children_idx_range) {}


    Tree Tree::build_tree(std::map<IdType, std::map<IdType, EdgeWeightType>> const& tree_map, IdType root_id) {
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


        // Calculate the sizes of the subtrees rooted at each vertex and store them in a two-dimensional vector.
        for (auto const& lvl : tree.levels) {
            tree.tree_sizes.emplace_back(lvl.size(), 1);
        }
        for (size_t lvl_idx = tree.levels.size() - 2; lvl_idx < tree.levels.size(); --lvl_idx) {
            size_t node_idx = 0;
            for (auto const& node : tree.levels[lvl_idx]) {
                for (size_t child_idx = node.children_idx_range.first; child_idx < node.children_idx_range.second; ++child_idx) {
                    tree.tree_sizes[lvl_idx][node_idx] += tree.tree_sizes[lvl_idx + 1][child_idx];
                }
                ++node_idx;
            }
        }

        return tree;
    }

    Tree Tree::build_tree(std::map<IdType, std::map<IdType, EdgeWeightType>> const& tree_map) {
        return build_tree(tree_map, tree_map.begin()->first);
    }

    SignaturesForTree Tree::cut(RationalType eps, SizeType part_cnt) {
        std::vector<std::vector<SignatureMap>> signatures;
        for (auto const& lvl : this->levels) {
            signatures.emplace_back(lvl.size());
        }

        // Calculate the size intervals of the connected components of a signature.
        std::vector<SizeType> const comp_size_bounds = calculate_upper_component_size_bounds(eps, this->tree_sizes[0][0], part_cnt);

        // Iterate over all nodes except the root starting with the node one the bottom left.
        for (size_t lvl_idx = this->levels.size() - 1; lvl_idx > 0; --lvl_idx) {
            for (size_t node_idx = 0; node_idx < this->levels[lvl_idx].size(); ++node_idx) {
                Node const& node = this->levels[lvl_idx][node_idx];
                bool const node_has_left_sibling = this->has_left_sibling[lvl_idx][node_idx];
                bool const node_has_child = node.children_idx_range.first < node.children_idx_range.second;
                SignatureMap empty_map;
                // The only signature which always has cut value smaller infinity(even if the node does not exist) is the 0-vector.
                empty_map[0][Signature(comp_size_bounds.size())] = 0;

                SignatureMap const* left_sibling_sigs = &empty_map;
                SignatureMap const* child_sigs = &empty_map;

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

                // Iterate over all calculated signatures of the left sibling and the rightmost child according
                // to the dynamic programming scheme described in the paper FF13.
                SignatureMap& node_sigs = signatures[lvl_idx][node_idx];
                for (auto const& left_sibling_sigs_with_size : *left_sibling_sigs) {
                    for (auto const& child_sigs_with_size : *child_sigs) {
                        for (auto const& left_sibling_sig : left_sibling_sigs_with_size.second) {
                            for (auto const& child_sig : child_sigs_with_size.second) {
                                // First case: The edge from the current node to its parent is not cut.
                                SizeType frontier_size = left_sibling_sigs_with_size.first + child_sigs_with_size.first;
                                EdgeWeightType cut_cost = left_sibling_sig.second + child_sig.second;
                                Signature sig = left_sibling_sig.first + child_sig.first;
                                if (node_sigs[frontier_size].find(sig) == node_sigs[frontier_size].end()) {
                                    node_sigs[frontier_size][sig] = cut_cost;
                                } else {
                                    node_sigs[frontier_size][sig] = std::min(node_sigs[frontier_size][sig], cut_cost);
                                }

                                // Second case: The edge from the current node to its parent is cut.
                                // Check if the current size of the component which includes the current node is smaller than
                                // the maximum allowed size.
                                SizeType const node_comp_size = this->tree_sizes[lvl_idx][node_idx] - child_sigs_with_size.first;
                                if (node_comp_size >= comp_size_bounds.back()) {
                                    continue;
                                } else {
                                    frontier_size += node_comp_size;
                                    cut_cost += node.parent_edge_weight;

                                    // Adjust the signature to account for the component which contains the current node.
                                    size_t i = 0; 
                                    while (node_comp_size >= comp_size_bounds[i]) { ++i; }
                                    sig[i] += 1;
                                    if (node_sigs[frontier_size].find(sig) == node_sigs[frontier_size].end()) {
                                        node_sigs[frontier_size][sig] = cut_cost;
                                    } else {
                                        node_sigs[frontier_size][sig] = std::min(node_sigs[frontier_size][sig], cut_cost);
                                    }
                                }
                            }
                        }
                    }
                }
            }

            // Calculate the signatures at the root according to the paper FF13. 
            // Signatures which contain less then the total amount of nodes are ignored.
            SignatureMap& root_sigs = signatures[0][0];
            SizeType const node_cnt = this->tree_sizes[0][0];
            for (auto const& sigs_with_size : signatures[1].back()) {
                SizeType const node_comp_size = node_cnt - sigs_with_size.first;
                if (node_comp_size >= comp_size_bounds.back()) {
                    continue;
                } else {
                    for (auto const& sig : sigs_with_size.second) {
                        Signature root_sig(sig.first);
                        size_t i = 0;
                        while(node_comp_size >= comp_size_bounds[i]) { ++i; }
                        root_sig[i] += 1;
                        if (root_sigs[node_cnt].find(root_sig) == root_sigs[node_cnt].end()) {
                            root_sigs[node_cnt][root_sig] = sig.second;
                        } else {
                            root_sigs[node_cnt][root_sig] = std::min(root_sigs[node_cnt][root_sig], sig.second);
                        }
                    }
                }
            }
        }

        return SignaturesForTree(part_cnt, eps, *this, std::move(signatures));
    }

    std::pair<size_t, size_t> Tree::get_node_idx(Node::IdType node_id) const {
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

    std::istream& operator>>(std::istream& is, Tree& tree) {
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
        tree = Tree::build_tree(tree_map, root_id);

        return is;
    }

    std::ostream& operator<<(std::ostream& os, Tree const& tree) {
        os << tree.tree_sizes[0][0] << " " << tree.levels[0][0].id << std::endl;
        for (size_t lvl_idx = 1; lvl_idx < tree.levels.size(); ++lvl_idx) {
            for (auto const& node : tree.levels[lvl_idx]) {
                os << tree.levels[lvl_idx - 1][node.parent_idx].id << " ";
                os << node.id << " " << node.parent_edge_weight << std::endl;
            }
        }
        return os;
    }

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

    std::vector<SizeType> calculate_lower_component_size_bounds(RationalType eps, SizeType node_cnt, SizeType part_cnt) {
        std::vector<SizeType> const upper_comp_size_bounds = calculate_upper_component_size_bounds(eps, node_cnt, part_cnt);
        std::vector<SizeType> lower_comp_size_bounds({1});
        lower_comp_size_bounds.insert(lower_comp_size_bounds.end(), upper_comp_size_bounds.begin(), upper_comp_size_bounds.end() - 1);

        return lower_comp_size_bounds;
    }

    SignaturesForTree::CutEdges SignaturesForTree::cut_edges_for_signature(Signature const& signature) const {
        using NodeIdx = std::pair<size_t, size_t>;

        struct NodeInfo {
            NodeIdx node_idx;
            Signature signature; 
            Node::EdgeWeightType cut_cost;
            SizeType remaining_node_cnt;

            NodeInfo(NodeIdx node_idx, Signature signature, Node::EdgeWeightType cut_cost, SizeType remaining_node_cnt) :
                node_idx(node_idx), signature(signature), cut_cost(cut_cost), remaining_node_cnt(remaining_node_cnt) {}
        };

        SizeType tree_size = this->tree.tree_sizes[0][0];
        Node::EdgeWeightType cut_cost = this->signatures[0][0].at(tree_size).at(signature);
        NodeInfo const root(NodeIdx(0, 0), signature, cut_cost, tree_size);
        NodeInfo root_right_child(NodeIdx(1, this->tree.levels[1].size() - 1), Signature(), cut_cost, 0);

        Signature curr_node_comp_sig(signature.size());
        Tree::SignatureMap const& root_right_child_sigs = 
            this->signatures[root_right_child.node_idx.first][root_right_child.node_idx.second];

        curr_node_comp_sig[0] = 1;
        size_t comp_size_bound_idx = 0;
        for (SizeType node_comp_size = 1; node_comp_size < this->upper_comp_size_bounds.back(); ++node_comp_size) {
            while (node_comp_size >= this->upper_comp_size_bounds[comp_size_bound_idx]) { 
                curr_node_comp_sig[comp_size_bound_idx] = 0;
                ++comp_size_bound_idx; 
                curr_node_comp_sig[comp_size_bound_idx] = 1;
            }

            SizeType const remaining_node_cnt = root.remaining_node_cnt - node_comp_size;
            if (root_right_child_sigs.find(remaining_node_cnt) != root_right_child_sigs.end()) {
                Signature const root_right_child_sig = root.signature - curr_node_comp_sig;
                auto const& signatures_w_size = root_right_child_sigs.at(remaining_node_cnt);
                if (signatures_w_size.find(root_right_child_sig) != signatures_w_size.end()) {
                    if (signatures_w_size.at(root_right_child_sig) == root.cut_cost) {
                        root_right_child.signature = root_right_child_sig;
                        root_right_child.remaining_node_cnt = remaining_node_cnt;
                        break;
                    }
                }
            }
        }

        CutEdges cut_edges;

        std::list<NodeInfo> queue = { root_right_child };
        while (!queue.empty()) {
            NodeInfo curr_info = queue.front();
            queue.pop_front();

            Node const& curr_node = this->tree.levels[curr_info.node_idx.first][curr_info.node_idx.second];

            Tree::SignatureMap empty_map; 
            empty_map[0][Signature(signature.size())] = 0;

            Tree::SignatureMap const* child_sigs = &empty_map;
            Tree::SignatureMap const* sibling_sigs = &empty_map;

            bool const curr_node_has_child = curr_node.children_idx_range.first < curr_node.children_idx_range.second;
            bool const curr_node_has_left_sibling = this->tree.has_left_sibling[curr_info.node_idx.first][curr_info.node_idx.second];

            if (curr_node_has_child) {
                child_sigs = &(this->signatures[curr_info.node_idx.first + 1][curr_node.children_idx_range.second - 1]);
            }
            if (curr_node_has_left_sibling) {
                sibling_sigs = &(this->signatures[curr_info.node_idx.first][curr_info.node_idx.second - 1]);
            }

            // Case 1: The edge from the current node to the parent node was not cut.
            for (auto const& child_sigs_with_size : *child_sigs) {
                SizeType const sibling_sig_size = curr_info.remaining_node_cnt - child_sigs_with_size.first;
                if (sibling_sigs->find(sibling_sig_size) != sibling_sigs->end()) {
                    for (auto const& child_sig : child_sigs_with_size.second) {
                        for (auto const& sibling_sig : sibling_sigs->at(sibling_sig_size)) {
                            if ((sibling_sig.first + child_sig.first == curr_info.signature).min() && 
                                    sibling_sig.second + child_sig.second == curr_info.cut_cost) {
                                if (curr_node_has_child) {
                                    queue.emplace_back(
                                            std::make_pair(curr_info.node_idx.first + 1, curr_node.children_idx_range.second - 1),
                                            child_sig.first,
                                            child_sig.second,
                                            child_sigs_with_size.first);

                                }
                                if (curr_node_has_left_sibling) {
                                    queue.emplace_back(
                                            std::make_pair(curr_info.node_idx.first, curr_info.node_idx.second - 1),
                                            sibling_sig.first,
                                            sibling_sig.second,
                                            sibling_sig_size);
                                }
                            }
                        }
                    }
                }
            }

            // Case 2: The edge was cut
            curr_node_comp_sig = 0;
            curr_node_comp_sig[0] = 1;
            comp_size_bound_idx = 0;
            for (SizeType node_comp_size = 1; node_comp_size < this->upper_comp_size_bounds.back(); ++node_comp_size) {

                while (node_comp_size >= this->upper_comp_size_bounds[comp_size_bound_idx]) { 
                    curr_node_comp_sig[comp_size_bound_idx] = 0;
                    ++comp_size_bound_idx; 
                    curr_node_comp_sig[comp_size_bound_idx] = 1;
                }


                for (auto const& child_sigs_with_size : *child_sigs) {
                    SizeType const sibling_sig_size = curr_info.remaining_node_cnt - child_sigs_with_size.first - node_comp_size;
                    if (sibling_sigs->find(sibling_sig_size) != sibling_sigs->end()) {
                        for (auto const& child_sig : child_sigs_with_size.second) {
                            for (auto const& sibling_sig : sibling_sigs->at(sibling_sig_size)) {
                                if ((sibling_sig.first + child_sig.first + curr_node_comp_sig == curr_info.signature).min() && 
                                        sibling_sig.second + child_sig.second + curr_node.parent_edge_weight == curr_info.cut_cost) {
                                    // The edge was cut and we found the signatures.
                                    if (curr_node_has_child) {
                                        queue.emplace_back(
                                                std::make_pair(curr_info.node_idx.first + 1, curr_node.children_idx_range.second - 1),
                                                child_sig.first,
                                                child_sig.second,
                                                child_sigs_with_size.first);

                                    }
                                    if (curr_node_has_left_sibling) {
                                        queue.emplace_back(
                                                std::make_pair(curr_info.node_idx.first, curr_info.node_idx.second - 1),
                                                sibling_sig.first,
                                                sibling_sig.second,
                                                sibling_sig_size);
                                    }

                                    cut_edges.insert(std::make_pair(curr_node.id, this->tree.levels[curr_info.node_idx.first - 1][curr_node.parent_idx].id));
                                }
                            }
                        }
                    }
                }

            }

        }
        return cut_edges;
    }

    std::vector<std::set<Node::IdType>> SignaturesForTree::components_for_cut_edges(CutEdges const& cut_edges) const {
        std::vector<std::set<Node::IdType>> components;

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
            Node const& curr_node = this->tree.levels[curr_info.node_idx.first][curr_info.node_idx.second];
            queue.pop_front();

            components[curr_info.component_idx].insert(curr_node.id);
            for (size_t child_idx = curr_node.children_idx_range.first; 
                    child_idx < curr_node.children_idx_range.second; ++child_idx) {
                Node const& child = this->tree.levels[curr_info.node_idx.first + 1][child_idx];
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

    std::ostream& operator<<(std::ostream& os, SignaturesForTree const& signatures) {
        os << signatures.part_cnt << " ";  
        os << signatures.eps.get_num() << " " << signatures.eps.get_den() << std::endl;
        os << std::endl;

        auto const& sigs = signatures.signatures;
        for (size_t lvl_idx = 0; lvl_idx < sigs.size(); ++lvl_idx) {
            for (size_t node_idx = 0; node_idx < sigs[lvl_idx].size(); ++node_idx) {
                auto const& node_sigs = sigs[lvl_idx][node_idx];
                os << signatures.tree.levels[lvl_idx][node_idx].id << " ";
                os << node_sigs.size() << std::endl;

                for (auto const& node_sigs_with_size : node_sigs) {
                    os << node_sigs_with_size.first << " ";
                    os << node_sigs_with_size.second.size() << std::endl;
                    for (auto const& sig : node_sigs_with_size.second) {
                        for (auto const& val : sig.first) {
                            os << val << " ";
                        }
                        os << sig.second << std::endl;
                    }
                }
                os << std::endl;
            }
        }
        return os;
    }

    SignaturesForTreeBuilder& SignaturesForTreeBuilder::with_part_cnt(SizeType part_cnt) {
        this->part_cnt = part_cnt;
        return *this;
    }

    SignaturesForTreeBuilder& SignaturesForTreeBuilder::with_eps(RationalType eps) {
        this->eps = eps;
        return *this;
    }

    SignaturesForTreeBuilder& SignaturesForTreeBuilder::with_signatures(std::vector<std::vector<Tree::SignatureMap>> const& signatures) {
        this->signatures = signatures;
        return *this;
    }

    SignaturesForTree SignaturesForTreeBuilder::finish() {
        return SignaturesForTree(this->part_cnt, this->eps, this->tree, this->signatures);
    }

    std::istream& operator>>(std::istream& is, SignaturesForTreeBuilder& builder) {
        SizeType part_cnt;
        int64_t eps_num;
        int64_t eps_denom;
        is >> part_cnt >> eps_num >> eps_denom;
        RationalType eps(eps_num, eps_denom);
        builder.with_part_cnt(part_cnt).with_eps(eps);
        auto signature_length = calculate_upper_component_size_bounds(
                eps, builder.tree.tree_sizes[0][0], part_cnt).size();

        std::vector<std::vector<Tree::SignatureMap>> signatures;
        for (auto const& level : builder.tree.levels) {
            signatures.emplace_back(level.size());
        }

        for (SizeType node_idx = 0; node_idx < builder.tree.tree_sizes[0][0]; ++node_idx) {
            Node::IdType node_id;
            SizeType size_cnt;
            is >> node_id >> size_cnt;
            auto node_idx_in_tree = builder.tree.get_node_idx(node_id);
            
            for (SizeType size_idx = 0; size_idx < size_cnt; ++size_idx) {
                SizeType size;
                SizeType signature_cnt;
                is >> size >> signature_cnt;
                for (SizeType signature_idx = 0; signature_idx < signature_cnt; ++signature_idx) {
                    std::valarray<SizeType> signature(signature_length);
                    Node::EdgeWeightType cut_cost;
                    for (auto& comp : signature) {
                        is >> comp;
                    }
                    is >> cut_cost;
                    signatures[node_idx_in_tree.first][node_idx_in_tree.second][size][signature] =
                        cut_cost;
                }
            }
        }
        builder.with_signatures(signatures);

        return is;
    }
}
