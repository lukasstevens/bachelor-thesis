#!/usr/bin/env bash
export CODE_DIR=$(cd ../../../.. && source code_dir.bash)
source ../../../../tables.bash

min_child=(2 3 4 5 7 9)
max_child=(3 4 5 6 8 10)
flags='-g tree_fat -m Tree_Partition -o time'
max_degree_table '--min_child' min_child '--max_child' max_child 100 2 1 3 20 "${flags}" &
max_degree_table '--min_child' min_child '--max_child' max_child 60 2 1 4 20 "${flags}" &
max_degree_table '--min_child' min_child '--max_child' max_child 60 3 1 3 20 "${flags}" &
wait
