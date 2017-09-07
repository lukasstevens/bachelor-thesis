#include<fstream>
#include<iostream>
#include<memory>
#include<sstream>

#include<gtest/gtest.h>

#include "Graph.hpp"
#include "GraphGen.hpp"


namespace nocontractimbalance {
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

    void imbalance_table(
            std::vector<graph::Rational> imbalances,
            std::string tree_gen_name,
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

            auto const comp_fn = [tree_gen_name, param] (std::stringstream& filename) {
                filename <<  "_" << tree_gen_name << "_";
                filename << "n" << param.node_cnt << "k" << param.kparts << ".dat";
            };
            comp_fn(metis_rec_filename);
            comp_fn(metis_kway_filename);
            comp_fn(kaffpa_filename);


            std::ofstream metis_rec_file(metis_rec_filename.str());
            std::ofstream metis_kway_file(metis_kway_filename.str());
            std::ofstream kaffpa_file(kaffpa_filename.str());

            auto print_header = [imbalances](std::ofstream& file) {
                file << "t";
                for(auto imbalance : imbalances) {
                    file << "\t" << imbalance;
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
                for (size_t idx = 0; idx < imbalances.size(); ++idx) {
                    graph::Graph<> orig_graph = (*orig_graph_gens.at(idx))(trie);
                    graph::Graph<> contract_graph = (*contract_graph_gens.at(idx))(trie);
                    graph::Graph<> tree = (*tree_gens.at(idx))(trie);

                    auto tree_part = tree.partition(param.kparts, imbalances.at(idx));
                    auto metis_rec_graph_part =
                        orig_graph.partition_metis_recursive(param.kparts, imbalances.at(idx));
                    auto metis_kway_graph_part =
                        orig_graph.partition_metis_kway(param.kparts, imbalances.at(idx));
                    auto kaffpa_graph_part =
                        orig_graph.partition_kaffpa(param.kparts, imbalances.at(idx));

                    double tree_part_cut_cost = 
                        contract_graph.partition_cost(tree.convert_part_to_node_repr(tree_part.second));
                    metis_rec_file << "\t" << (tree_part_cut_cost / metis_rec_graph_part.first);
                    metis_kway_file << "\t" << (tree_part_cut_cost / metis_kway_graph_part.first);
                    kaffpa_file << "\t" << (tree_part_cut_cost / kaffpa_graph_part.first);
                }
                metis_rec_file << "\n";
                metis_kway_file << "\n";
                kaffpa_file << "\n";
            }
        }
    }

    int32_t node_count = 80;

    std::vector<Params> params({
            Params(node_count, 2, graph::Rational(1, 1)),
            Params(node_count, 3, graph::Rational(1, 1)),
            Params(node_count, 4, graph::Rational(1, 1))
            });

    std::vector<graph::Rational> imbalances({
            graph::Rational(8, 20), graph::Rational(7, 20),
            graph::Rational(6, 20), graph::Rational(5, 20)
            });


    TEST(DISABLED_GraphEdgeProb1, RstImbalance) {

        std::vector<IGraphGenPtr> graph_gens;
        std::vector<IGraphGenPtr> contract_gens;
        std::vector<IGraphGenPtr> tree_gens;
        for (auto imbalance : imbalances) {
            IGraphGenPtr graph_gen(new graphgen::GraphEdgeProb<>(node_count, 0.1));
            IGraphGenPtr contract_gen = graph_gen;
            IGraphGenPtr tree_gen(new graphgen::Rst<>(graph_gen));
            graph_gens.push_back(graph_gen);
            contract_gens.push_back(contract_gen);
            tree_gens.push_back(tree_gen);
        }

        imbalance_table(
                imbalances,
                "edge_prob1_rst",
                params,
                graph_gens,
                contract_gens,
                tree_gens,
                10
                );
    }

    TEST(DISABLED_GraphEdgeProb3, RstImbalance) {

        std::vector<IGraphGenPtr> graph_gens;
        std::vector<IGraphGenPtr> contract_gens;
        std::vector<IGraphGenPtr> tree_gens;
        for (auto imbalance : imbalances) {
            IGraphGenPtr graph_gen(new graphgen::GraphEdgeProb<>(node_count, 0.3));
            IGraphGenPtr contract_gen = graph_gen;
            IGraphGenPtr tree_gen(new graphgen::Rst<>(graph_gen));
            graph_gens.push_back(graph_gen);
            contract_gens.push_back(contract_gen);
            tree_gens.push_back(tree_gen);
        }

        imbalance_table(
                imbalances,
                "edge_prob3_rst",
                params,
                graph_gens,
                contract_gens,
                tree_gens,
                10
                );
    }

    TEST(DISABLED_GraphEdgeProb1, MstImbalance) {

        std::vector<IGraphGenPtr> graph_gens;
        std::vector<IGraphGenPtr> contract_gens;
        std::vector<IGraphGenPtr> tree_gens;
        for (auto imbalance : imbalances) {
            IGraphGenPtr graph_gen(new graphgen::GraphEdgeProb<>(node_count, 0.1));
            IGraphGenPtr contract_gen = graph_gen;
            IGraphGenPtr tree_gen(new graphgen::Mst<>(graph_gen));
            graph_gens.push_back(graph_gen);
            contract_gens.push_back(contract_gen);
            tree_gens.push_back(tree_gen);
        }

        imbalance_table(
                imbalances,
                "edge_prob1_mst",
                params,
                graph_gens,
                contract_gens,
                tree_gens,
                10
                );
    }

    TEST(DISABLED_GraphEdgeProb3, MstImbalance) {

        std::vector<IGraphGenPtr> graph_gens;
        std::vector<IGraphGenPtr> contract_gens;
        std::vector<IGraphGenPtr> tree_gens;
        for (auto imbalance : imbalances) {
            IGraphGenPtr graph_gen(new graphgen::GraphEdgeProb<>(node_count, 0.3));
            IGraphGenPtr contract_gen = graph_gen;
            IGraphGenPtr tree_gen(new graphgen::Mst<>(graph_gen));
            graph_gens.push_back(graph_gen);
            contract_gens.push_back(contract_gen);
            tree_gens.push_back(tree_gen);
        }

        imbalance_table(
                imbalances,
                "edge_prob3_mst",
                params,
                graph_gens,
                contract_gens,
                tree_gens,
                10
                );
    }

    TEST(DISABLED_GraphEdgeProb1, DecompImbalance) {

        std::vector<IGraphGenPtr> graph_gens;
        std::vector<IGraphGenPtr> contract_gens;
        std::vector<IGraphGenPtr> tree_gens;
        for (auto imbalance : imbalances) {
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

        imbalance_table(
                imbalances,
                "edge_prob1_decomp",
                params,
                graph_gens,
                contract_gens,
                tree_gens,
                10
                );
    }

    TEST(DISABLED_GraphEdgeProb3, DecompImbalance) {

        std::vector<IGraphGenPtr> graph_gens;
        std::vector<IGraphGenPtr> contract_gens;
        std::vector<IGraphGenPtr> tree_gens;
        for (auto imbalance : imbalances) {
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

        imbalance_table(
                imbalances,
                "edge_prob3_decomp",
                params,
                graph_gens,
                contract_gens,
                tree_gens,
                10
                );
    }

    TEST(DISABLED_GraphPrefAttach10, RstImbalance) {

        std::vector<IGraphGenPtr> graph_gens;
        std::vector<IGraphGenPtr> contract_gens;
        std::vector<IGraphGenPtr> tree_gens;
        for (auto imbalance : imbalances) {
            IGraphGenPtr graph_gen(new graphgen::GraphPrefAttach<>(node_count, 10));
            IGraphGenPtr contract_gen = graph_gen;
            IGraphGenPtr tree_gen(new graphgen::Rst<>(graph_gen));
            graph_gens.push_back(graph_gen);
            contract_gens.push_back(contract_gen);
            tree_gens.push_back(tree_gen);
        }

        imbalance_table(
                imbalances,
                "pref_attach10_rst",
                params,
                graph_gens,
                contract_gens,
                tree_gens,
                10
                );
    }

    TEST(DISABLED_GraphPrefAttach5, RstImbalance) {

        std::vector<IGraphGenPtr> graph_gens;
        std::vector<IGraphGenPtr> contract_gens;
        std::vector<IGraphGenPtr> tree_gens;
        for (auto imbalance : imbalances) {
            IGraphGenPtr graph_gen(new graphgen::GraphPrefAttach<>(node_count, 5));
            IGraphGenPtr contract_gen = graph_gen;
            IGraphGenPtr tree_gen(new graphgen::Rst<>(graph_gen));
            graph_gens.push_back(graph_gen);
            contract_gens.push_back(contract_gen);
            tree_gens.push_back(tree_gen);
        }

        imbalance_table(
                imbalances,
                "pref_attach5_rst",
                params,
                graph_gens,
                contract_gens,
                tree_gens,
                10
                );
    }

    TEST(DISABLED_GraphPrefAttach10, MstImbalance) {

        std::vector<IGraphGenPtr> graph_gens;
        std::vector<IGraphGenPtr> contract_gens;
        std::vector<IGraphGenPtr> tree_gens;
        for (auto imbalance : imbalances) {
            IGraphGenPtr graph_gen(new graphgen::GraphPrefAttach<>(node_count, 10));
            IGraphGenPtr contract_gen = graph_gen;
            IGraphGenPtr tree_gen(new graphgen::Mst<>(graph_gen));
            graph_gens.push_back(graph_gen);
            contract_gens.push_back(contract_gen);
            tree_gens.push_back(tree_gen);
        }

        imbalance_table(
                imbalances,
                "pref_attach10_mst",
                params,
                graph_gens,
                contract_gens,
                tree_gens,
                10
                );
    }

    TEST(DISABLED_GraphPrefAttach5, MstImbalance) {

        std::vector<IGraphGenPtr> graph_gens;
        std::vector<IGraphGenPtr> contract_gens;
        std::vector<IGraphGenPtr> tree_gens;
        for (auto imbalance : imbalances) {
            IGraphGenPtr graph_gen(new graphgen::GraphPrefAttach<>(node_count, 5));
            IGraphGenPtr contract_gen = graph_gen;
            IGraphGenPtr tree_gen(new graphgen::Mst<>(graph_gen));
            graph_gens.push_back(graph_gen);
            contract_gens.push_back(contract_gen);
            tree_gens.push_back(tree_gen);
        }

        imbalance_table(
                imbalances,
                "pref_attach5_mst",
                params,
                graph_gens,
                contract_gens,
                tree_gens,
                10
                );
    }

    TEST(DISABLED_GraphPrefAttach10, DecompImbalance) {

        std::vector<IGraphGenPtr> graph_gens;
        std::vector<IGraphGenPtr> contract_gens;
        std::vector<IGraphGenPtr> tree_gens;
        for (auto imbalance : imbalances) {
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

        imbalance_table(
                imbalances,
                "pref_attach10_decomp",
                params,
                graph_gens,
                contract_gens,
                tree_gens,
                10
                );
    }

    TEST(DISABLED_GraphPrefAttach5, DecompImbalance) {

        std::vector<IGraphGenPtr> graph_gens;
        std::vector<IGraphGenPtr> contract_gens;
        std::vector<IGraphGenPtr> tree_gens;
        for (auto imbalance : imbalances) {
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

        imbalance_table(
                imbalances,
                "pref_attach5_decomp",
                params,
                graph_gens,
                contract_gens,
                tree_gens,
                10
                );
    }
}
