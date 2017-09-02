#include<chrono>
#include<functional>
#include<iostream>
#include<limits>
#include<memory>
#include<string>

#include<args.hxx>

#include "GraphGen.hpp"
#include "GraphIo.hpp"

enum Output {
    GRAPHVIZ_GRAPH_PARTITION,
    PARTITION,
    GRAPH,
    TIME,
    CUT_COST,
    GRAPHVIZ_GRAPH,
    DECOMP_GRAPH
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
    std::chrono::milliseconds time_elapsed;

    Result() = default;

    Result(
            std::string method_name,
            graph::PartitionResult<int, int> part_result,
            std::chrono::milliseconds time_elapsed
          ) : 
        method_name(method_name), part_result(part_result),
        time_elapsed(time_elapsed) {}
};


Result run_part_method(
        std::string method_name,
        std::function<graph::PartitionResult<int,int>()> method
        ) {
    using namespace std::chrono;
    auto start = steady_clock::now();
    auto method_res = method();
    auto end = steady_clock::now();
    milliseconds time_elapsed =
        duration_cast<milliseconds>(end - start);
    return Result(method_name, method_res, time_elapsed);
}

void run_gen_group(
        std::shared_ptr<graphgen::IGraphGen<>> const& generator,
        int kparts,
        graph::Rational imbalance, 
        std::vector<PartMethods> part_methods,
        std::vector<Output> output,
        size_t seed,
        size_t tries
        ) {

    if (part_methods.size() > 0 && kparts == 0) {
        throw args::ValidationError(
                "kparts is required if a partition method is applied.");
    }

    for (size_t trie_idx = 0; trie_idx < tries; ++trie_idx) {
        graph::Graph<> graph = (*generator)(seed + trie_idx);
        std::vector<Result> results;
        for (auto method : part_methods) {
            switch (method) {
                case TREE_PARTITION:
                    results.push_back(run_part_method(
                                "Tree_Partition",
                                [graph, kparts, imbalance](){
                                return graph.partition(kparts, imbalance);
                                })
                            );
                    break;
                case METIS_KWAY:
                    results.push_back(run_part_method(
                                "METIS_Kway",
                                [graph, kparts, imbalance](){
                                return graph.partition_metis_kway(kparts, imbalance);
                                })
                            );
                    break;
                case METIS_REC:
                    results.push_back(run_part_method(
                                "METIS_Recursive",
                                [graph, kparts, imbalance](){
                                return graph.partition_metis_recursive(kparts, imbalance);
                                })
                            );
                    break;
                case KAFFPA:
                    results.push_back(run_part_method(
                                "KaFFPa",
                                [graph, kparts, imbalance](){
                                return graph.partition_kaffpa(kparts, imbalance);
                                })
                            );
                    break;
            }
        }

        for (auto output_method : output) {
            switch (output_method) {
                case GRAPHVIZ_GRAPH:
                    std::cout << graphio::PrintGraphviz<>(graph);
                    std::cout << std::endl;
                    break;
                case GRAPHVIZ_GRAPH_PARTITION:
                    for (auto const& result : results) {
                        std::cout << graphio::PrintGraphviz<>(graph, result.part_result.second);
                        std::cout << std::endl;
                    }
                    std::cout << std::endl;
                    break;
                case DECOMP_GRAPH:
                    std::cout << graphio::PrintDecompFmt<>(graph, false);
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
                    for (auto const& result : results) {
                        std::cout << result.time_elapsed.count() << "\t";
                    }
                    std::cout << std::endl;
                    break;
                case CUT_COST:
                    for (auto const& result : results) {
                        std::cout << result.part_result.first << "\t";
                    }
                    std::cout << std::endl;
                    break;
                case GRAPH:
                    std::cout << graph;
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
            {"graphviz_graph_part", Output::GRAPHVIZ_GRAPH_PARTITION},
            {"part", Output::PARTITION},
            {"time", Output::TIME},
            {"cut_cost", Output::CUT_COST},
            {"graphviz_graph", Output::GRAPHVIZ_GRAPH},
            {"decomp_graph", Output::DECOMP_GRAPH},
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
        TREE_PREF_ATTACH,
        TREE_FAT,
        GRAPH_PREF_ATTACH,
        GRAPH_EDGE_PROB
    };
    std::unordered_map<std::string, GraphGen> graph_gen_map({
            {"tree_rand_attach", GraphGen::TREE_RAND_ATTACH},
            {"tree_pref_attach", GraphGen::TREE_PREF_ATTACH},
            {"tree_fat", GraphGen::TREE_FAT},
            {"graph_pref_attach", GraphGen::GRAPH_PREF_ATTACH},
            {"graph_edge_prob", GraphGen::GRAPH_EDGE_PROB}
            });
    std::string graph_gen_options("OPTIONS:");
    for (auto const& option : graph_gen_map) {
        graph_gen_options += " " + option.first;
    }
    args::MapFlag<std::string, GraphGen> graph_gen(
            gen_group, "graph_gen", "Graph generator to use. " + graph_gen_options,
            {'g', "generator"}, graph_gen_map);

    args::ValueFlag<int> node_count(
            gen_group, "node count", "The number of nodes when using a graph generator.",
            {'n', "nodes"});

    args::ValueFlag<int> edge_count(
            gen_group, "edge count",
            "The number of edges when using the graph_pref_attach generator.",
            {'e', "edges"});

    args::ValueFlag<double> edge_prob(
            gen_group, "edge probability",
            "The probability that an edge exists when using the graph_egde_prob generator.",
            {"edge_prob"});

    args::Group degree_group(
            gen_group,
            "Child count range for tree_fat, max_degree for everything else.",
            args::Group::Validators::Xor);
    args::ValueFlag<int> max_degree(
            degree_group, "max degree", "The maximum degree of a node when using a graph generator.",
            {'d', "max_degree"}, std::numeric_limits<int>::max());
    args::Group child_count_group(
            degree_group,
            "Lower and upper exclusive child count must be specified.",
            args::Group::Validators::All
            );
    args::ValueFlag<int> min_child_cnt(
            child_count_group, "min child count", "The minimum number of childs in a fat tree.",
            {"min_child"});
    args::ValueFlag<int> max_child_cnt(
            child_count_group, "max child count", "The maximum number of childs in a fat tree.",
            {"max_child"});


    args::ValueFlag<int> kparts(
            parser, "kparts", "The number of parts to partition into.",
            {'k', "kparts"}, 0);
    args::ValueFlag<graph::Rational> imbalance(
            parser, "imbalance", "The allowed imbalance of a partition: "
            "each part has at most (1+imbalance)*ceil(node_count/kparts) nodes.",
            {'i', "imbalance"},
            graph::Rational(1,2)
            );

    args::ValueFlag<size_t> tries(
            gen_group, "tries", "Number of different graphs to generate.",
            {'t', "tries"}, 1);

    args::ValueFlag<size_t> seed(
            gen_group, "seed", "The initial seed to use for generating graphs.",
            {'s', "seed"}, 0);

    try
    {
        parser.ParseCLI(argc, argv);

        if (gen_group) {
            if(!graph_gen) {
                throw args::ValidationError("Graph generator required");
            }

            std::shared_ptr<graphgen::IGraphGen<>> generator(nullptr);
            switch (args::get(graph_gen)) {
                case TREE_RAND_ATTACH:
                    generator =
                        std::shared_ptr<graphgen::IGraphGen<>>(
                                new graphgen::TreeRandAttach<>(
                                    args::get(node_count),
                                    args::get(max_degree)
                                    )
                                );
                    break;
                case TREE_PREF_ATTACH:
                    generator =
                        std::shared_ptr<graphgen::IGraphGen<>>(
                                new graphgen::TreePrefAttach<>(
                                    args::get(node_count),
                                    args::get(max_degree)
                                    )
                                );
                    break;
                case TREE_FAT:
                    generator = 
                        std::shared_ptr<graphgen::IGraphGen<>>(
                                new graphgen::TreeFat<>(
                                    args::get(node_count),
                                    std::make_pair(
                                        args::get(min_child_cnt),
                                        args::get(max_child_cnt)
                                        )
                                    )
                                );
                    break;
                case GRAPH_EDGE_PROB:
                    generator =
                        std::shared_ptr<graphgen::IGraphGen<>>(
                                new graphgen::GraphEdgeProb<>(
                                    args::get(node_count),
                                    args::get(edge_prob),
                                    args::get(max_degree)
                                    )
                                );
                    break;
                case GRAPH_PREF_ATTACH:
                    generator =
                        std::shared_ptr<graphgen::IGraphGen<>>(
                                new graphgen::GraphPrefAttach<>(
                                    args::get(node_count),
                                    args::get(edge_count),
                                    args::get(max_degree)
                                    )
                                );
                    break;
            }

            run_gen_group(
                    generator,
                    args::get(kparts),
                    args::get(imbalance),
                    args::get(part_methods),
                    args::get(output),
                    args::get(seed),
                    args::get(tries)
                    );
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
