#include<fstream>
#include<sstream>

#include<gtest/gtest.h>

#include "Graph.hpp"
#include "GraphGen.hpp"

namespace datasets {

    void dataset_table(
            std::string dataset_name,
            std::vector<int32_t> kparts,
            std::vector<graph::Rational> imbalances
            ) {
        std::string dataset_w_pre = "resources/data_sets/" + dataset_name + "/" + dataset_name;
        graph::Graph<> orig_graph =
            graphgen::FromFile<>(dataset_w_pre + ".graph")();
        graph::Graph<> contracted_graph =
            graphgen::FromFile<>(dataset_w_pre + "_contracted.graph")();
        graph::Graph<> decomp_graph = (graphgen::ContractInfEdges<>(
                std::shared_ptr<graphgen::IGraphGen<>>(
                        new graphgen::FromFile<>(dataset_w_pre + "_decomposed.graph")
                        )
                ))();

        std::ofstream metis_rec_file("METIS_Recursive_" + dataset_name + ".dat");
        std::ofstream metis_kway_file("METIS_Kway_" + dataset_name + ".dat");
        std::ofstream kaffpa_file("KaFFPa_" + dataset_name + ".dat");

        std::vector<std::ofstream*> files({
                &metis_rec_file, &metis_kway_file, &kaffpa_file
                });
        
        auto print_header = [kparts](std::ofstream* file){
            *file << "ik";
            for (auto k : kparts) {
                *file << "\t" << k;
            }
            *file << "\n";
        };
        for (auto file : files) {
            print_header(file);
        }

        for (auto imbalance : imbalances) {
            for (auto file : files) {
                *file << imbalance;
            }

            std::cerr << imbalance;
            for (auto k : kparts) {
                std::cerr << " " << k;

                auto tree_part_res = decomp_graph.partition(k, imbalance);
                auto metis_rec_res = orig_graph.partition_metis_recursive(k, imbalance);
                auto metis_kway_res = orig_graph.partition_metis_kway(k, imbalance);
                auto kaffpa_res = orig_graph.partition_kaffpa(k, imbalance);

                double tree_part_cost = 
                    contracted_graph.partition_cost(decomp_graph.convert_part_to_node_repr(tree_part_res.second));
                for (auto file : files) {
                    *file << "\t";
                }
                metis_rec_file << (tree_part_cost / metis_rec_res.first);
                metis_kway_file << (tree_part_cost / metis_kway_res.first);
                kaffpa_file << (tree_part_cost / kaffpa_res.first);
            }

            for (auto file : files) {
                *file << "\n";
            }
        }
    }

    std::vector<int32_t> kparts({
            2, 3, 4, 6
            });

    std::vector<graph::Rational> imbalances({
            graph::Rational(8, 20), graph::Rational(7, 20),
            graph::Rational(6, 20), graph::Rational(5, 20)
            });

    TEST(DISABLED_Dataset, as) {
        dataset_table("as19990829", kparts, imbalances);
    }

    TEST(DISABLED_Dataset, facebook) {
        dataset_table("ego-Facebook", std::vector<int32_t>({2, 3, 4}), imbalances);
    }

    TEST(DISABLED_Dataset, caGrQc) {
        dataset_table("ca-GrQc", std::vector<int32_t>({2, 3, 4}), imbalances);
    }

    TEST(DISABLED_Dataset, email ) {
        dataset_table("email-Eu-core", kparts, imbalances);
    }
}
