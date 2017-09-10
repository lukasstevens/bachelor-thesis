#include<fstream>
#include<iostream>
#include<memory>
#include<sstream>
#include<thread>

#include<gtest/gtest.h>

#include "Graph.hpp"
#include "GraphGen.hpp"

namespace nocontractnodecount {

    // Check if partition violates balance constraints
    bool violates_max_part_size(std::vector<int32_t> partition, int32_t kparts, graph::Rational imbalance) {
        int32_t max_part_size = gmputils::floor_to_int<int32_t>((graph::Rational(1) + imbalance) *
                gmputils::ceil_to_int<int32_t>(graph::Rational(partition.size(), kparts)));

        std::vector<int32_t> part_size(static_cast<size_t>(kparts));
        for (auto const in_part : partition) {
            part_size.at(static_cast<size_t>(in_part)) += 1;
        }

        return std::any_of(part_size.cbegin(), part_size.cend(),
                [max_part_size](int32_t const part_size){ return part_size > max_part_size; });
    }

    using IGraphGen = graphgen::IGraphGen<>;
    using IGraphGenPtr = std::shared_ptr<IGraphGen>;

    struct Params {
        public:
            int32_t node_cnt;
            int32_t kparts;
            graph::Rational imbalance;

            Params(int32_t node_cnt, int32_t kparts, graph::Rational imbalance) :
                node_cnt(node_cnt), kparts(kparts), imbalance(imbalance) {}
    };

    void node_count_table(
            std::vector<int32_t> const& node_counts,
            std::string const& tree_gen_name,
            std::vector<Params> const& params,
            std::vector<IGraphGenPtr> const& orig_graph_gens,
            std::vector<IGraphGenPtr> const& contract_graph_gens,
            std::vector<IGraphGenPtr> const& tree_gens,
            size_t tries
            ) {

        for (auto const& param : params) {
            std::stringstream metis_rec_filename;
            metis_rec_filename << "METIS_Recursive";
            std::stringstream metis_kway_filename;
            metis_kway_filename << "METIS_Kway";
            std::stringstream kaffpa_filename;
            kaffpa_filename << "KaFFPa";

            auto const comp_fn = [tree_gen_name, param, node_counts](std::stringstream& filename) {
                filename <<  "_" << tree_gen_name << "_";
                filename << "k" << param.kparts << "i" << param.imbalance.get_num()
                    << "div" << param.imbalance.get_den() << ".dat";
            };
            comp_fn(metis_rec_filename);
            comp_fn(metis_kway_filename);
            comp_fn(kaffpa_filename);


            std::ofstream metis_rec_file(metis_rec_filename.str());
            std::ofstream metis_kway_file(metis_kway_filename.str());
            std::ofstream kaffpa_file(kaffpa_filename.str());

            auto print_header = [node_counts](std::ofstream& file) {
                file << "t";
                for(auto node_count : node_counts) {
                    file << "\t" << node_count;
                }
                file << "\n";
            };
            print_header(metis_rec_file);
            print_header(metis_kway_file);
            print_header(kaffpa_file);

            for (size_t trie = 0; trie < tries; ++trie) {
                metis_rec_file << trie;
                metis_kway_file << trie;
                kaffpa_file << trie;
                for (size_t idx = 0; idx < node_counts.size(); ++idx) {
                    graph::Graph<> orig_graph = (*orig_graph_gens.at(idx))(trie);
                    graph::Graph<> contract_graph = (*contract_graph_gens.at(idx))(trie);
                    graph::Graph<> tree = (*tree_gens.at(idx))(trie);

                    auto tree_part = tree.partition(param.kparts, param.imbalance);
                    auto metis_rec_graph_part =
                        orig_graph.partition_metis_recursive(param.kparts, param.imbalance);
                    auto metis_kway_graph_part =
                        orig_graph.partition_metis_kway(param.kparts, param.imbalance);
                    auto kaffpa_graph_part =
                        orig_graph.partition_kaffpa(param.kparts, param.imbalance);

                    int32_t tree_part_cut_cost = 
                        contract_graph.partition_cost(tree.convert_part_to_node_repr(tree_part.second));

                    if (metis_rec_graph_part.first < tree_part_cut_cost && 
                            !violates_max_part_size(metis_rec_graph_part.second, param.kparts, param.imbalance)) {
                        std::cerr << "metis_rec does not violate and is better for n:" << node_counts.at(idx); 
                        std::cerr << ",k:" << param.kparts << ",i:" << param.imbalance << std::endl;
                    }
                    if (metis_kway_graph_part.first < tree_part_cut_cost && 
                            !violates_max_part_size(metis_kway_graph_part.second, param.kparts, param.imbalance)) {
                        std::cerr << "metis_kway does not violate and is better for n:" << node_counts.at(idx); 
                        std::cerr << ",k:" << param.kparts << ",i:" << param.imbalance << std::endl;
                    }
                    if (kaffpa_graph_part.first < tree_part_cut_cost && 
                            !violates_max_part_size(kaffpa_graph_part.second, param.kparts, param.imbalance)) {
                        std::cerr << "kaffpa does not violate and is better for n:" << node_counts.at(idx); 
                        std::cerr << ",k:" << param.kparts << ",i:" << param.imbalance << std::endl;
                    }

                    double tree_part_cut_cost_d = 
                        contract_graph.partition_cost(tree.convert_part_to_node_repr(tree_part.second));
                    metis_rec_file << "\t" << (tree_part_cut_cost_d / metis_rec_graph_part.first);
                    metis_kway_file << "\t" << (tree_part_cut_cost_d / metis_kway_graph_part.first);
                    kaffpa_file << "\t" << (tree_part_cut_cost_d / kaffpa_graph_part.first);
                }
                metis_rec_file << "\n";
                metis_kway_file << "\n";
                kaffpa_file << "\n";
            }
        }
    }

    std::vector<Params> params({
            Params(-1, 2, graph::Rational(1, 3)),
            Params(-1, 4, graph::Rational(1, 3)),
            Params(-1, 6, graph::Rational(1, 3))
            });

    std::vector<int32_t> node_counts({
            70, 80, 90, 100
            });

    TEST(DISABLED_GraphEdgeProb1, RstNodeCount) {

        std::vector<IGraphGenPtr> graph_gens;
        std::vector<IGraphGenPtr> contract_gens;
        std::vector<IGraphGenPtr> tree_gens;
        for (auto node_count : node_counts) {
            IGraphGenPtr graph_gen(new graphgen::GraphEdgeProb<>(node_count, 0.1));
            IGraphGenPtr contract_gen = graph_gen;
            IGraphGenPtr tree_gen(new graphgen::Rst<>(graph_gen));
            graph_gens.push_back(graph_gen);
            contract_gens.push_back(contract_gen);
            tree_gens.push_back(tree_gen);
        }

        node_count_table(
                node_counts,
                "edge_prob1_rst",
                params,
                graph_gens,
                contract_gens,
                tree_gens,
                10
                );
    }

    TEST(DISABLED_GraphEdgeProb3, RstNodeCount) {

        std::vector<IGraphGenPtr> graph_gens;
        std::vector<IGraphGenPtr> contract_gens;
        std::vector<IGraphGenPtr> tree_gens;
        for (auto node_count : node_counts) {
            IGraphGenPtr graph_gen(new graphgen::GraphEdgeProb<>(node_count, 0.3));
            IGraphGenPtr contract_gen = graph_gen;
            IGraphGenPtr tree_gen(new graphgen::Rst<>(graph_gen));
            graph_gens.push_back(graph_gen);
            contract_gens.push_back(contract_gen);
            tree_gens.push_back(tree_gen);
        }

        node_count_table(
                node_counts,
                "edge_prob3_rst",
                params,
                graph_gens,
                contract_gens,
                tree_gens,
                10
                );
    }

    TEST(DISABLED_GraphEdgeProb1, MstNodeCount) {

        std::vector<IGraphGenPtr> graph_gens;
        std::vector<IGraphGenPtr> contract_gens;
        std::vector<IGraphGenPtr> tree_gens;
        for (auto node_count : node_counts) {
            IGraphGenPtr graph_gen(new graphgen::GraphEdgeProb<>(node_count, 0.1));
            IGraphGenPtr contract_gen = graph_gen;
            IGraphGenPtr tree_gen(new graphgen::Mst<>(graph_gen));
            graph_gens.push_back(graph_gen);
            contract_gens.push_back(contract_gen);
            tree_gens.push_back(tree_gen);
        }

        node_count_table(
                node_counts,
                "edge_prob1_mst",
                params,
                graph_gens,
                contract_gens,
                tree_gens,
                10
                );
    }

    TEST(DISABLED_GraphEdgeProb3, MstNodeCount) {

        std::vector<IGraphGenPtr> graph_gens;
        std::vector<IGraphGenPtr> contract_gens;
        std::vector<IGraphGenPtr> tree_gens;
        for (auto node_count : node_counts) {
            IGraphGenPtr graph_gen(new graphgen::GraphEdgeProb<>(node_count, 0.3));
            IGraphGenPtr contract_gen = graph_gen;
            IGraphGenPtr tree_gen(new graphgen::Mst<>(graph_gen));
            graph_gens.push_back(graph_gen);
            contract_gens.push_back(contract_gen);
            tree_gens.push_back(tree_gen);
        }

        node_count_table(
                node_counts,
                "edge_prob3_mst",
                params,
                graph_gens,
                contract_gens,
                tree_gens,
                10
                );
    }

    TEST(DISABLED_GraphEdgeProb1, DecompNodeCount) {

        std::vector<IGraphGenPtr> graph_gens;
        std::vector<IGraphGenPtr> contract_gens;
        std::vector<IGraphGenPtr> tree_gens;
        for (auto node_count : node_counts) {
            IGraphGenPtr graph_gen(new graphgen::GraphEdgeProb<>(node_count, 0.1));
            IGraphGenPtr contract_gen = graph_gen;
            std::stringstream filename;
            filename << "resources/decomp_trees/";
            filename << "edge_prob1n" << node_count << ".graph";
            IGraphGenPtr tree_gen(new graphgen::ContractInfEdges<>(
                        IGraphGenPtr(new graphgen::FromFile<>(filename.str(), 10))));
            graph_gens.push_back(graph_gen);
            contract_gens.push_back(contract_gen);
            tree_gens.push_back(tree_gen);
        }

        node_count_table(
                node_counts,
                "edge_prob1_decomp",
                params,
                graph_gens,
                contract_gens,
                tree_gens,
                10
                );
    }

    TEST(DISABLED_GraphEdgeProb3, DecompNodeCount) {

        std::vector<IGraphGenPtr> graph_gens;
        std::vector<IGraphGenPtr> contract_gens;
        std::vector<IGraphGenPtr> tree_gens;
        for (auto node_count : node_counts) {
            IGraphGenPtr graph_gen(new graphgen::GraphEdgeProb<>(node_count, 0.3));
            IGraphGenPtr contract_gen = graph_gen;
            std::stringstream filename;
            filename << "resources/decomp_trees/";
            filename << "edge_prob3n" << node_count << ".graph";
            IGraphGenPtr tree_gen(new graphgen::ContractInfEdges<>(
                        IGraphGenPtr(new graphgen::FromFile<>(filename.str(), 10))));
            graph_gens.push_back(graph_gen);
            contract_gens.push_back(contract_gen);
            tree_gens.push_back(tree_gen);
        }

        node_count_table(
                node_counts,
                "edge_prob3_decomp",
                params,
                graph_gens,
                contract_gens,
                tree_gens,
                10
                );
    }

    TEST(DISABLED_GraphPrefAttach10, RstNodeCount) {

        std::vector<IGraphGenPtr> graph_gens;
        std::vector<IGraphGenPtr> contract_gens;
        std::vector<IGraphGenPtr> tree_gens;
        for (auto node_count : node_counts) {
            IGraphGenPtr graph_gen(new graphgen::GraphPrefAttach<>(node_count, 10));
            IGraphGenPtr contract_gen = graph_gen;
            IGraphGenPtr tree_gen(new graphgen::Rst<>(graph_gen));
            graph_gens.push_back(graph_gen);
            contract_gens.push_back(contract_gen);
            tree_gens.push_back(tree_gen);
        }

        node_count_table(
                node_counts,
                "pref_attach10_rst",
                params,
                graph_gens,
                contract_gens,
                tree_gens,
                10
                );
    }

    TEST(DISABLED_GraphPrefAttach5, RstNodeCount) {

        std::vector<IGraphGenPtr> graph_gens;
        std::vector<IGraphGenPtr> contract_gens;
        std::vector<IGraphGenPtr> tree_gens;
        for (auto node_count : node_counts) {
            IGraphGenPtr graph_gen(new graphgen::GraphPrefAttach<>(node_count, 5));
            IGraphGenPtr contract_gen = graph_gen;
            IGraphGenPtr tree_gen(new graphgen::Rst<>(graph_gen));
            graph_gens.push_back(graph_gen);
            contract_gens.push_back(contract_gen);
            tree_gens.push_back(tree_gen);
        }

        node_count_table(
                node_counts,
                "pref_attach5_rst",
                params,
                graph_gens,
                contract_gens,
                tree_gens,
                10
                );
    }

    TEST(DISABLED_GraphPrefAttach10, MstNodeCount) {

        std::vector<IGraphGenPtr> graph_gens;
        std::vector<IGraphGenPtr> contract_gens;
        std::vector<IGraphGenPtr> tree_gens;
        for (auto node_count : node_counts) {
            IGraphGenPtr graph_gen(new graphgen::GraphPrefAttach<>(node_count, 10));
            IGraphGenPtr contract_gen = graph_gen;
            IGraphGenPtr tree_gen(new graphgen::Mst<>(graph_gen));
            graph_gens.push_back(graph_gen);
            contract_gens.push_back(contract_gen);
            tree_gens.push_back(tree_gen);
        }

        node_count_table(
                node_counts,
                "pref_attach10_mst",
                params,
                graph_gens,
                contract_gens,
                tree_gens,
                10
                );
    }

    TEST(DISABLED_GraphPrefAttach5, MstNodeCount) {

        std::vector<IGraphGenPtr> graph_gens;
        std::vector<IGraphGenPtr> contract_gens;
        std::vector<IGraphGenPtr> tree_gens;
        for (auto node_count : node_counts) {
            IGraphGenPtr graph_gen(new graphgen::GraphPrefAttach<>(node_count, 5));
            IGraphGenPtr contract_gen = graph_gen;
            IGraphGenPtr tree_gen(new graphgen::Mst<>(graph_gen));
            graph_gens.push_back(graph_gen);
            contract_gens.push_back(contract_gen);
            tree_gens.push_back(tree_gen);
        }

        node_count_table(
                node_counts,
                "pref_attach5_mst",
                params,
                graph_gens,
                contract_gens,
                tree_gens,
                10
                );
    }

    TEST(DISABLED_GraphPrefAttach10, DecompNodeCount) {

        std::vector<IGraphGenPtr> graph_gens;
        std::vector<IGraphGenPtr> contract_gens;
        std::vector<IGraphGenPtr> tree_gens;
        for (auto node_count : node_counts) {
            IGraphGenPtr graph_gen(new graphgen::GraphPrefAttach<>(node_count, 10));
            IGraphGenPtr contract_gen = graph_gen;
            std::stringstream filename;
            filename << "resources/decomp_trees/";
            filename << "pref_attach10n" << node_count << ".graph";
            IGraphGenPtr tree_gen(new graphgen::ContractInfEdges<>(
                        IGraphGenPtr(new graphgen::FromFile<>(filename.str(), 10))));
            graph_gens.push_back(graph_gen);
            contract_gens.push_back(contract_gen);
            tree_gens.push_back(tree_gen);
        }

        node_count_table(
                node_counts,
                "pref_attach10_decomp",
                params,
                graph_gens,
                contract_gens,
                tree_gens,
                10
                );
    }

    TEST(DISABLED_GraphPrefAttach5, DecompNodeCount) {

        std::vector<IGraphGenPtr> graph_gens;
        std::vector<IGraphGenPtr> contract_gens;
        std::vector<IGraphGenPtr> tree_gens;
        for (auto node_count : node_counts) {
            IGraphGenPtr graph_gen(new graphgen::GraphPrefAttach<>(node_count, 5));
            IGraphGenPtr contract_gen = graph_gen;
            std::stringstream filename;
            filename << "resources/decomp_trees/";
            filename << "pref_attach5n" << node_count << ".graph";
            IGraphGenPtr tree_gen(new graphgen::ContractInfEdges<>(
                        IGraphGenPtr(new graphgen::FromFile<>(filename.str(), 10))));
            graph_gens.push_back(graph_gen);
            contract_gens.push_back(contract_gen);
            tree_gens.push_back(tree_gen);
        }

        node_count_table(
                node_counts,
                "pref_attach5_decomp",
                params,
                graph_gens,
                contract_gens,
                tree_gens,
                10
                );
    }

    TEST(DISABLED_Trees, NodeCount) {
        std::vector<std::string> tree_gen_names({
                "rand_attach", "pref_attach", "fat", "bnary"
                });
        for (auto const& gen_name : tree_gen_names) {
            std::vector<IGraphGenPtr> graph_gens;
            for (size_t i = 0; i < node_counts.size(); ++i) {
                IGraphGenPtr graph_gen;
                if (gen_name == "rand_attach") {
                    graph_gen =
                        IGraphGenPtr(new graphgen::TreeRandAttach<>(node_counts.at(i)));
                } else if (gen_name == "pref_attach") {
                    graph_gen =
                        IGraphGenPtr(new graphgen::TreePrefAttach<>(node_counts.at(i)));
                } else if (gen_name == "fat") {
                    graph_gen =
                        IGraphGenPtr(new graphgen::TreeFat<>(node_counts.at(i), std::make_pair(2, 11)));
                } else if (gen_name == "bnary") {
                    graph_gen =
                        IGraphGenPtr(new graphgen::TreeFat<>(node_counts.at(i), std::make_pair(2, 3)));
                }
                graph_gens.push_back(graph_gen);
            }
           node_count_table(
                    node_counts,
                    gen_name,
                    params,
                    graph_gens,
                    graph_gens,
                    graph_gens,
                    20
                    );
        }
    }
}
