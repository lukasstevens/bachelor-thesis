#!/usr/bin/env bash
export CODE_DIR=$(cd ../../../.. && source code_dir.bash)
source ../../../../tables.bash
source ../params.bash

flags='-g tree_rand_attach -m Tree_Partition -o time'
kparts_table kparts ${first[n]} ${first[ie]} ${first[id]} ${tries} "${flags}" &
kparts_table kparts ${second[n]} ${second[ie]} ${second[id]} ${tries} "${flags}" &
kparts_table kparts ${third[n]} ${third[ie]} ${third[id]} ${tries} "${flags}" &
wait
