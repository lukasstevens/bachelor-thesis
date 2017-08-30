#!/usr/bin/env bash
export CODE_DIR=$(cd ../../../.. && source code_dir.bash)
source ../../../../tables.bash

kparts=(3 4 5 6 8 10)
flags='-g tree_fat -m Tree_Partition -o time --min_child=2 --max_child=3'
kparts_table kparts 50 3 10 20 "${flags}" &
kparts_table kparts 50 1 4 20 "${flags}" &
kparts_table kparts 50 1 5 20 "${flags}" &
wait
