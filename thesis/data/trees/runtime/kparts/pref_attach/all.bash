#!/usr/bin/env bash
export CODE_DIR=$(cd ../../../.. && source code_dir.bash)
source ../../../../tables.bash

kparts=(3 4 5 6 8 10)
flags='-g tree_pref_attach -m Tree_Partition -o time'
kparts_table kparts 50 3 10 20 "${flags}" &
kparts_table kparts 50 1 4 20 "${flags}" &
kparts_table kparts 50 1 5 20 "${flags}" &
wait
