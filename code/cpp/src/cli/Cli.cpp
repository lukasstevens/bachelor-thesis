#include<iostream>
#include<limits>
#include<string>

#include<args.hxx>

#include "GraphGen.hpp"
#include "GraphIo.hpp"

enum Output {
    GRAPH_PARTITION,
    PARTITION,
    TIME,
    CUT_COST,
    GRAPH
};

enum PartMethods {
    TREE_PARTITION,
    METIS_KWAY,
    METIS_REC,
    KAFFPA
};

struct Result {
    std::string method_name;
    graph::PartitionResult<int, int> part_result;

    Result(
            std::string method_name,
            graph::PartitionResult<int, int> part_result
          ) : 
        method_name(method_name), part_result(part_result) {}
};

void execute_partitioning(
        graphgen::IGraphGen<>* generator,
        int kparts,
        graph::Rational imbalance, 
        std::vector<PartMethods> part_methods,
        std::vector<Output> output,
        size_t seed=0,
        size_t tries=1
        ) {
   
    for (size_t trie_idx = 0; trie_idx < tries; ++trie_idx) {
        graph::Graph<> graph = (*generator)(seed + trie_idx);
        std::vector<Result> results;
        for (auto method : part_methods) {
            switch (method) {
                case TREE_PARTITION:
                    results.emplace_back(
                            "Tree_Partition", graph.partition(kparts, imbalance));
                    break;
                case METIS_KWAY:
                    results.emplace_back(
                            "METIS_Kway", graph.partition_metis_kway(kparts, imbalance));
                    break;
                case METIS_REC:
                    results.emplace_back(
                            "METIS_Recursive", graph.partition_metis_recursive(kparts, imbalance));
                    break;
                case KAFFPA:
                    results.emplace_back(
                            "KaFFPa", graph.partition_kaffpa(kparts, imbalance));
                    break;
            }
        }

        for (auto output_method : output) {
            switch (output_method) {
                case GRAPH:
                    std::cout << graph::PrintGraphviz<>(graph);
                    std::cout << std::endl;
                    break;
                case GRAPH_PARTITION:
                    for (auto const& result : results) {
                        std::cout << graph::PrintGraphviz<>(graph, result.part_result.second);
                        std::cout << std::endl;
                    }
                    std::cout << std::endl;
                    break;
                case PARTITION:
                    for (auto const& result : results) {
                        for (auto const part_idx : result.part_result.second) {
                            std::cout << part_idx << "\n";
                        }
                        std::cout << std::endl;
                    }
                    std::cout << std::endl;
                    break;
                case TIME:
                    break;
                case CUT_COST:
                    for (auto const& result : results) {
                        std::cout << result.part_result.first << "\t";
                    }
                    std::cout << std::endl;
                    break;
            }
        }
    }
}


int main(int argc, char** argv) {
    args::ArgumentParser parser(
            "Tree partitioning using dynamic programming.",
            "Lukas Stevens"
            );

    args::HelpFlag help(parser, "help", "Display this help menu.", {'h', "help"});

    std::unordered_map<std::string, PartMethods> part_method_map({
            {"Tree_Partition", PartMethods::TREE_PARTITION},
            {"METIS_Kway", PartMethods::METIS_KWAY},
            {"METIS_Recursive", PartMethods::METIS_REC},
            {"KaFFPa", PartMethods::KAFFPA},
            });
    std::string part_method_options("OPTIONS:");
    for (auto const& option : part_method_map) {
        part_method_options += " " + option.first;
    }
    args::MapFlagList<std::string, PartMethods> part_methods(
            parser,
            "method",
            "Partitioning methods to use. " + part_method_options,
            {'m', "method"},
            part_method_map
            );

    std::unordered_map<std::string, Output> output_map({
            {"graph_part", Output::GRAPH_PARTITION},
            {"part", Output::PARTITION},
            {"time", Output::TIME},
            {"cut_cost", Output::CUT_COST},
            {"graph", Output::GRAPH}
            });
    std::string output_options("OPTIONS:");
    for (auto const& option : output_map) {
        output_options += " " + option.first;
    }
    args::MapFlagList<std::string, Output> output(
            parser, "output", "Data to ouput. " + output_options,
            {'o', "output"}, output_map);

    args::Group gen_or_files(parser, "Either generator or input files.",
            args::Group::Validators::Xor);
    args::Group gen_group(gen_or_files, "Graph generators.",
            args::Group::Validators::DontCare);

    enum GraphGen {
        TREE_RAND_ATTACH,
        TREE_PREF_ATTACH
    };
    std::unordered_map<std::string, GraphGen> graph_gen_map({
            {"tree_rand_attach", GraphGen::TREE_RAND_ATTACH},
            {"tree_pref_attach", GraphGen::TREE_PREF_ATTACH}
            });
    std::string graph_gen_options("OPTIONS:");
    for (auto const& option : graph_gen_map) {
        graph_gen_options += " " + option.first;
    }
    args::MapFlag<std::string, GraphGen> graph_gen(
            gen_group, "graph_gen", "Graph generator to use. " + graph_gen_options,
            {'g', "generator"}, graph_gen_map);

    args::ValueFlag<int> node_count(
            gen_group, "node count", "The number of nodes when using a graph generator",
            {'n', "nodes"});
    args::ValueFlag<int> max_degree(
            gen_group, "max degree", "The maximum degree of a node when using a graph generator",
            {'d', "max_degree"});

    args::ValueFlag<int> kparts(
            parser, "kparts", "The number of parts to partition into.",
            {'k', "kparts"});
    args::ValueFlag<graph::Rational> imbalance(
            parser, "imbalance", "The allowed imbalance of a partition: "
            "each part has at most (1+imbalance)*ceil(node_count/kparts) nodes.",
            {'i', "imbalance"});

    try
    {
        parser.ParseCLI(argc, argv);
        if (!kparts || !imbalance) {
            throw args::ValidationError("kparts and imbalance is required.");
        }

        if (gen_group) {
            if(!graph_gen) {
                throw args::ValidationError("Graph generator required.");
            }
            int max_degree_int = std::numeric_limits<int>::max();
            if (max_degree) {
                max_degree_int = args::get(max_degree);
            }

            graphgen::IGraphGen<>* generator = nullptr;
            switch (args::get(graph_gen)) {
                case TREE_RAND_ATTACH:
                    generator =
                        new graphgen::TreeRandAttach<>(
                                args::get(node_count),
                                max_degree_int
                                );
                    break;
                case TREE_PREF_ATTACH:
                    generator =
                        new graphgen::TreePrefAttach<>(
                                args::get(node_count),
                                max_degree_int
                                );
                    break;
            }

            execute_partitioning(
                    generator,
                    args::get(kparts),
                    args::get(imbalance),
                    args::get(part_methods),
                    args::get(output)
                    );

            delete generator;
        }
    }
    catch (args::Help)
    {
        std::cout << parser;
        return 0;
    }
    catch (args::ParseError e)
    {
        std::cerr << e.what() << std::endl;
        std::cerr << parser;
        return 1;
    }
    return 0;
}
