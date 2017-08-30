#!/usr/bin/env bash
export CODE_DIR=$(cd ../../../.. && source code_dir.bash)
source ../../../../tables.bash

node_counts=(70 90 110 130)
flags='-g tree_fat -m Tree_Partition -o time --min_child=2 --max_child=11'
node_count_table node_counts 2 1 3 20 "${flags}" &
node_count_table node_counts 3 1 3 20 "${flags}" &
node_count_table node_counts 2 1 4 20 "${flags}" &
wait
