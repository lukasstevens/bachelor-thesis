#!/usr/bin/env bash
export CODE_DIR=$(cd ../../../.. && source code_dir.bash)
source ../../../../tables.bash

imbalance=('7/20' '3/10' '5/20' '2/10')
flags='-g tree_fat -m Tree_Partition -o time --min_child=2 --max_child=11'
imbalance_table imbalance 50 2 20 "${flags}" &
imbalance_table imbalance 100 2 20 "${flags}" &
imbalance_table imbalance 50 4 20 "${flags}" &
wait
