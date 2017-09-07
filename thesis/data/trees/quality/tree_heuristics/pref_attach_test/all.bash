#!/usr/bin/env bash
export CODE_DIR=$(cd ../../../.. && source code_dir.bash)
source ../../../../tables.bash

flags=(-g tree_pref_attach)
method_table 70 2 1 3 50 flags &
method_table 70 4 1 3 50 flags &
method_table 70 6 1 3 50 flags &
method_table 70 8 1 3 50 flags &
wait
