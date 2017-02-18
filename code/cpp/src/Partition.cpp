#include<iostream>
#include<cstdint>
#include<vector>
#include<unordered_map>
#include<unordered_set>
#include<string>
#include<sstream>
#include<list>

typedef uint32_t u32;
typedef int32_t i32;


struct Node {
    public:
        u32 id;
        Node* parent;
        i32 parent_edge_weight;
        Node* children;
        u32 children_cnt;

        Node() : id(0), parent(nullptr), children(nullptr), children_cnt(0) {}
        Node(u32 i, Node* p, i32 w): id(i), parent(p), parent_edge_weight(w), children(nullptr), children_cnt(0) {}

        ~Node() {
            delete[] children;
        }

        static Node* build_tree(std::unordered_map<u32, std::unordered_map<u32, i32>>& graph) {
            Node* root = new Node(graph.begin()->first, nullptr, 0);

            std::list<std::pair<u32, Node*>> queue;
            queue.push_back(std::make_pair(0, root));
            std::unordered_set<u32> visited;

            while (!queue.empty()) {
                auto node_pair = queue.front();
                queue.pop_front();
                Node* curr_node = &node_pair.second[node_pair.first];
                visited.insert(curr_node->id);

                curr_node->children_cnt = static_cast<u32>(graph[curr_node->id].size());
                if (curr_node->parent != nullptr) {
                    --curr_node->children_cnt;
                }
                curr_node->children = new Node[curr_node->children_cnt]; 

                u32 idx = 0;
                for (auto neighbor : graph[curr_node->id]) {
                    if (visited.find(neighbor.first) == visited.end()) {
                        curr_node->children[idx] = Node(neighbor.first, curr_node, neighbor.second);
                        queue.push_back(std::make_pair(idx, curr_node->children));
                        idx += 1;
                    }
                }
            }
            return root;
        }

        std::string to_string() {
            std::stringstream stream;
            std::list<Node*> queue;
            queue.push_back(this);

            while (!queue.empty()) {
                Node* curr_node = queue.front();
                queue.pop_front();

                for (u32 child_idx = 0; child_idx < curr_node->children_cnt; ++child_idx) {
                    stream << curr_node->id << " -> " << curr_node->children[child_idx].id << "[label=\"" << curr_node->children[child_idx].parent_edge_weight << "\"]\n";
                    queue.push_back(&curr_node->children[child_idx]);
                }
            }
            return stream.str();
        }
};

int main() {

    u32 node_cnt;
    std::cin >> node_cnt;

    std::unordered_map<u32, std::unordered_map<u32, i32>> graph;
    for (u32 edge_idx = 0; edge_idx < node_cnt - 1; ++edge_idx) {
        u32 from;
        u32 to;
        i32 weight;
        std::cin >> from >> to >> weight;
        graph[from][to] = weight;
        graph[to][from] = weight;
    }

    Node* root = Node::build_tree(graph);
    std::cerr << root->to_string() << std::endl;
    delete(root);
}
