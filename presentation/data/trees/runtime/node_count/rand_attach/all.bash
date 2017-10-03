#!/usr/bin/env bash
export CODE_DIR=$(cd ../../../.. && source code_dir.bash)
source ../../../../tables.bash

node_counts=(70 90 110 130)
node_count_table node_counts 2 1 3 20 '-g tree_rand_attach -m Tree_Partition -o time' & 
node_count_table node_counts 3 1 3 20 '-g tree_rand_attach -m Tree_Partition -o time' &
node_count_table node_counts 2 1 4 20 '-g tree_rand_attach -m Tree_Partition -o time' &
wait
