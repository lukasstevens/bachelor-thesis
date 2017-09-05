#include<chrono>
#include<cstdint>
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
    GRAPHVIZ_GRAPH
};

enum OutputMod {
    ORIG_GRAPH,
    TREE,
    BOTH
};

enum PartMethods {
    TREE_PARTITION,
    METIS_KWAY,
    METIS_REC,
    KAFFPA
};

struct Result {
    std::string method_name;
    graph::PartitionResult<int32_t, int32_t> part_result;
    std::chrono::milliseconds time_elapsed;

    Result() = default;

    Result(
            std::string method_name,
            graph::PartitionResult<int32_t, int32_t> part_result,
            std::chrono::milliseconds time_elapsed
          ) : 
        method_name(method_name), part_result(part_result),
        time_elapsed(time_elapsed) {}
};


Result run_part_method(
        std::string method_name,
        std::function<graph::PartitionResult<int32_t,int32_t>()> method
        ) {
    using namespace std::chrono;
    auto start = steady_clock::now();
    auto method_res = method();
    auto end = steady_clock::now();
    milliseconds time_elapsed =
        duration_cast<milliseconds>(end - start);
    return Result(method_name, method_res, time_elapsed);
}

using IGraphGen = std::shared_ptr<graphgen::IGraphGen<>>;

void run(
        std::vector<IGraphGen> tree_part_graphs,
        std::vector<IGraphGen> orig_graphs,
        int32_t kparts,
        graph::Rational imbalance, 
        std::vector<PartMethods> part_methods,
        std::vector<Output> output,
        OutputMod output_mod,
        size_t seed,
        size_t tries
        ) {

    if (part_methods.size() > 0 && kparts == 0) {
        throw args::ValidationError(
                "kparts is required if a partition method is applied.");
    }

    for (size_t graph_idx = 0; graph_idx < orig_graphs.size(); ++graph_idx) {
        for (size_t trie_idx = 0; trie_idx < tries; ++trie_idx) {
            graph::Graph<> graph = (*orig_graphs.at(graph_idx))(seed + trie_idx);
            graph::Graph<> tree_part_graph = (*tree_part_graphs.at(graph_idx))(seed + trie_idx); 
            std::vector<Result> results;
            for (auto method : part_methods) {
                switch (method) {
                    case TREE_PARTITION:
                        results.push_back(run_part_method(
                                    "Tree_Partition",
                                    [tree_part_graph, kparts, imbalance](){
                                    return tree_part_graph.partition(kparts, imbalance);
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
                        if (output_mod != OutputMod::TREE) {
                            std::cout << graphio::PrintGraphviz<>(graph);
                            std::cout << std::endl;
                        }
                        if (output_mod != OutputMod::ORIG_GRAPH) {
                            std::cout << graphio::PrintGraphviz<>(tree_part_graph);
                            std::cout << std::endl;
                        }
                        break;
                    case GRAPHVIZ_GRAPH_PARTITION:
                        for (auto const& result : results) {
                            if (result.method_name == "Tree_Partition") {
                                std::cout << graphio::PrintGraphviz<>(tree_part_graph, result.part_result.second);
                            } else {
                                std::cout << graphio::PrintGraphviz<>(graph, result.part_result.second);
                            }
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
                        if (output_mod != OutputMod::TREE) {
                            std::cout << graph;
                            std::cout << std::endl;
                        }
                        if (output_mod != OutputMod::ORIG_GRAPH) {
                            std::cout << tree_part_graph;
                            std::cout << std::endl;
                        }
                        break;
                }
            }
        }
    }
}

template<typename E>
std::string option_string(std::unordered_map<std::string, E> const& map) {
    std::string options("OPTIONS:");
    for (auto const& option : map) {
        options += " " + option.first;
    }
    return options;
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
            {"graph", Output::GRAPH}
            });
    args::MapFlagList<std::string, Output> output(
            parser, "output", "Data to ouput. " + option_string(output_map),
            {'o', "output"}, output_map);

    std::unordered_map<std::string, OutputMod> output_mod_map({
            {"graph", OutputMod::ORIG_GRAPH},
            {"tree", OutputMod::TREE},
            {"both", OutputMod::BOTH},
            });
    args::MapFlag<std::string, OutputMod> output_mod(
            parser, "output modifier",
            "Output original graph, tree (converted from graph) or both. DEFAULT: graph. " +
            option_string(output_mod_map),
            {"output_mod"}, output_mod_map, OutputMod::ORIG_GRAPH);

    args::Group gen_or_files_group(parser, "Either generator or input files.",
            args::Group::Validators::DontCare);
    args::Group gen_group(gen_or_files_group, "Graph generators.",
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
    args::MapFlagList<std::string, GraphGen> graph_gen_list(
            gen_group, "graph_gen", "Graph generator to use. " + option_string(graph_gen_map),
            {'g', "generator"}, graph_gen_map);

    args::ValueFlag<int32_t> node_count(
            gen_group, "node count", "The number of nodes when using a graph generator.",
            {'n', "nodes"});

    args::ValueFlag<int32_t> edge_cnt_p_node(
            gen_group, "edge count per node",
            "The number of edges per node when using the graph_pref_attach generator.",
            {'e', "edges_p_node"});

    args::ValueFlag<double> edge_prob(
            gen_group, "edge probability",
            "The probability that an edge exists when using the graph_egde_prob generator.",
            {"edge_prob"});

    args::Group degree_group(
            gen_group,
            "Child count range for tree_fat, max_degree for everything else.",
            args::Group::Validators::Xor);
    args::ValueFlag<int32_t> max_degree(
            degree_group, "max degree", "The maximum degree of a node when using a graph generator.",
            {'d', "max_degree"}, std::numeric_limits<int32_t>::max());
    args::Group child_count_group(
            degree_group,
            "Lower and upper exclusive child count must be specified.",
            args::Group::Validators::All
            );
    args::ValueFlag<int32_t> min_child_cnt(
            child_count_group, "min child count", "The minimum number of childs in a fat tree.",
            {"min_child"});
    args::ValueFlag<int32_t> max_child_cnt(
            child_count_group, "max child count", "The maximum number of childs in a fat tree.",
            {"max_child"});


    args::ValueFlag<int32_t> kparts(
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

    args::Group prep_group(
            parser, "Preprocessing step for tree partition",
            args::ArgumentParser::Validators::DontCare);
    enum Prep {
        MST,
        RST,
        CONTRACT,
        CONTRACT_INF
    };
    std::unordered_map<std::string, Prep> prep_map({
            {"mst", Prep::MST},
            {"rst", Prep::RST},
            {"contract", Prep::CONTRACT},
            {"contract_inf", Prep::CONTRACT_INF}
            });
    args::MapFlagList<std::string, Prep> prep_list(
            prep_group, "Preprocessing", "Preprocessing step for tree partition. " + option_string(prep_map),
            {'p', "tree_prep"}, prep_map);
    args::ValueFlag<int32_t> contract_to_flag(
            prep_group, "Number of nodes to contract to",
            "Number of nodes to contract to when using contract preprocessing.",
            {"contract_to"});


    args::Group file_group(
            gen_or_files_group, "Use Input files", args::Group::Validators::AtLeastOne);
    args::ValueFlagList<std::string> file_list(
            file_group, "files", "File for graph input or - for stdin.",
            {'f', "file"});
    args::ValueFlagList<std::string> tree_file_list(
            file_group, "tree files", "Read a tree from file. For every tree file there must be a graph file."
            " The tree files are read first then the graph files.",
            {"tree_file"});
    args::ValueFlagList<std::string> graph_file_list(
            file_group, "graph files", "Read a graph from file.",
            {"graph_file"});

    try
    {
        parser.ParseCLI(argc, argv);

        std::vector<IGraphGen> tree_part_graphs;
        std::vector<IGraphGen> orig_graphs;

        if(!gen_or_files_group) {
            throw args::ValidationError("Graph generator or file input required.");
        }

        for (auto gen : args::get(graph_gen_list)) {
            IGraphGen generator;
            switch (gen) {
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
                                    args::get(edge_cnt_p_node),
                                    args::get(max_degree)
                                    )
                                );
                    break;
            }
            tree_part_graphs.push_back(generator);
            orig_graphs.push_back(generator);
        }

        if (file_list) {
            for (auto file : args::get(file_list)) {
                tree_part_graphs.emplace_back(new graphgen::FromFile<>(file));
                auto graph = (*tree_part_graphs.back())();
                orig_graphs.emplace_back(new graphgen::GraphId<>(graph));
            }
        }

        if (tree_file_list) {
            auto tree_files = args::get(tree_file_list);
            auto graph_files = args::get(graph_file_list);
            if (tree_files.size() != graph_files.size()) {
                throw args::ValidationError("Need equal number of tree files and graph files");
            }

            for (auto const& tree_file : tree_files) {
                tree_part_graphs.emplace_back(new graphgen::FromFile<>(tree_file));
            }
            for (auto const& graph_file : graph_files) {
                orig_graphs.emplace_back(new graphgen::FromFile<>(graph_file));
            }
        }

        if (prep_list) {
            for (auto prep : args::get(prep_list)) {
                switch (prep) {
                    case MST:
                        for (auto& tree : tree_part_graphs) {
                            tree = std::shared_ptr<graphgen::IGraphGen<>>(
                                    new graphgen::Mst<>(tree));
                        }
                        break;
                    case RST:
                        for (auto& tree : tree_part_graphs) {
                            tree = std::shared_ptr<graphgen::IGraphGen<>>(
                                    new graphgen::Rst<>(tree));
                        }
                        break;
                    case CONTRACT:
                        for (auto& tree : tree_part_graphs) {
                            tree = std::shared_ptr<graphgen::IGraphGen<>>(
                                    new graphgen::ContractToN<>(tree, args::get(contract_to_flag)));
                        }
                        break;
                    case CONTRACT_INF:
                        for (auto& tree : tree_part_graphs) {
                            tree = std::shared_ptr<graphgen::IGraphGen<>>(
                                    new graphgen::ContractInfEdges<>(tree));
                        }
                        break;
                }
            }
        }

        run(
                tree_part_graphs,
                orig_graphs,
                args::get(kparts),
                args::get(imbalance), 
                args::get(part_methods),
                args::get(output),
                args::get(output_mod),
                args::get(seed),
                args::get(tries)
           );
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
