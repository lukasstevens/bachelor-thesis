#pragma once

#include<cstdint>
#include<map>
#include<unordered_map>
#include<string>
#include<vector>

namespace part {

    struct Signature {
        public:
        using CountType = uint32_t;

        std::vector<CountType> sig;

        Signature(std::vector<CountType>);

        friend bool operator==(const Signature& lhs, const Signature& rhs);
        friend inline bool operator!=(const Signature& lhs, const Signature& rhs) {
            return !(lhs == rhs);
        }

        Signature& operator+=(const Signature& rhs);
        friend Signature operator+(Signature lhs, const Signature& rhs);
    };

}

namespace std {

    template <>
    struct hash<part::Signature> {
        size_t operator()(const part::Signature& s) const;
    };

}

namespace part {

    struct Node {
        public:
        using IdType = uint32_t;
        using EdgeWeightType = int32_t;

        IdType const id;
        Node* const parent;
        EdgeWeightType const parent_edge_weight;
        IdType subtree_size;
        std::vector<Node> children;

        Node(IdType, Node*, EdgeWeightType);
        Node(Node&&) = default;
        Node& operator=(Node&&) = default;
        // Delete copy constructor and copy-assignment so that no accidental copies happen.
        Node(Node const&) = delete;
        Node& operator=(Node const&) = delete;

        static Node build_tree(std::unordered_map<IdType, std::unordered_map<IdType, EdgeWeightType>>& tree, IdType root_id);
        static Node build_tree(std::unordered_map<IdType, std::unordered_map<IdType, EdgeWeightType>>& tree);

        std::string to_string();
    };

    struct Tree {
        public:
        using SignatureMap = std::map<Signature::CountType, std::unordered_map<Signature, Signature::CountType>>;

        std::vector<std::vector<Node>> levels;
        std::vector<std::vector<SignatureMap>> signatures;
    };
}

