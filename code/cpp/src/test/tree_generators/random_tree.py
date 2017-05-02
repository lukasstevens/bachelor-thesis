#!/usr/bin/env python3
import random
import sys

def generate_random_tree(node_count, edge_weight_range=range(1, 101)):
    graph = {0: {}}
    for idx in range(1, node_count):
        parent_node = random.choice(graph)
        parent_node[idx] = random.choice(edge_weight_range)
        graph[idx] = {}
    return graph

def main():
    tree = {}
    if len(sys.argv) == 2:
        tree = generate_random_tree(node_count=int(sys.argv[1]))
    else:
        print("Error: You have to specify the number of nodes")

    # The root node is always 0
    print(str(len(tree)) + " 0")
    for (from_id, to_ids) in tree.items():
        for (to_id, weight) in to_ids.items():
            print("{0:d} {1:d} {2:d}".format(from_id, to_id, weight))

if __name__ == "__main__":
    main()
