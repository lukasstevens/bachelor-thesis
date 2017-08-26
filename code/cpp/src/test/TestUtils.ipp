#include<fstream>
#include<sstream>

#include<gtest/gtest.h>

namespace testutils {

    template<typename SizeType>
        AlgorithmParams<SizeType> get_algorithm_params(std::string tree_name, std::string params_name) {
            SizeType part_cnt;
            long eps_num;
            long eps_denom;
            std::string params_resource_name("resources/" + tree_name + "." + params_name + ".signatures");
            std::ifstream param_stream(params_resource_name);
            EXPECT_TRUE(param_stream.good()) << "Failed to open file " << params_resource_name << ".";
            param_stream >> part_cnt >> eps_num >> eps_denom;
            param_stream.close();
            cut::Rational eps(eps_num, eps_denom);
            return AlgorithmParams<SizeType>(eps, part_cnt);
        }

    template<typename IdType, typename EdgeWeightType>
        cut::Tree<IdType, EdgeWeightType> get_tree(std::string tree_name) {
            std::string tree_resource_filename = "resources/" + tree_name + ".tree";
            std::ifstream tree_stream(tree_resource_filename);
            EXPECT_TRUE(tree_stream.good()) << "Failed to open file " << tree_resource_filename << ".";
            cut::Tree<IdType, EdgeWeightType> tree;
            tree_stream >> tree;
            tree_stream.close();
            return tree;
        }

    template<typename IdType, typename NodeWeight, typename EdgeWeightType>
        cut::SignaturesForTree<IdType, NodeWeight, EdgeWeightType> get_signatures_for_tree(std::string tree_name, 
                std::string params_name, cut::Tree<IdType, NodeWeight, EdgeWeightType> const& tree) {

            std::string signatures_resource = "resources/" + tree_name + "." + params_name + ".signatures";
            std::ifstream signature_stream(signatures_resource);
            EXPECT_TRUE(signature_stream.good()) << "Failed to open file " << signatures_resource << ".";
            cut::SignaturesForTreeBuilder<IdType, NodeWeight, EdgeWeightType> signature_builder(tree);
            signature_stream >> signature_builder;
            signature_stream.close();
            return signature_builder.finish();
        }

    template<typename IdType, typename EdgeWeightType>
        std::tuple<part::Partitioning<IdType>, cut::Signature<IdType>, EdgeWeightType> get_opt_partitioning(
                std::string tree_name,
                std::string params_name) {

            std::string partitioning_name = "resources/" + tree_name + "." + params_name + ".parts";
            std::ifstream partitioning_stream(partitioning_name);
            EXPECT_TRUE(partitioning_stream.good());

            std::string line;

            std::vector<IdType> best_signature;
            {
                std::getline(partitioning_stream, line);
                std::stringstream line_stream(line);

                IdType comp_cnt;
                while(line_stream >> comp_cnt) {
                    best_signature.push_back(comp_cnt);
                }
            }

            std::getline(partitioning_stream, line);
            EdgeWeightType opt_cut_cost = std::stoi(line);

            std::getline(partitioning_stream, line);
            size_t part_cnt = std::stoul(line);
            part::Partitioning<IdType> partitioning(part_cnt);
            for (size_t part_idx = 0; part_idx < part_cnt; ++part_idx) {
                std::getline(partitioning_stream, line);
                std::stringstream line_stream(line);
                IdType node_id;
                while(line_stream >> node_id) {
                    partitioning[part_idx].insert(node_id);
                }
            }
            
            partitioning_stream.close();
            return std::make_tuple(
                    partitioning, 
                    cut::Signature<IdType>(&best_signature.front(), best_signature.size()),
                    opt_cut_cost);
        }
}
