#!/usr/bin/env bash
export CODE_DIR=$(cd ../../../.. && source code_dir.bash)
source ../../../../tables.bash

flags=(-g tree_pref_attach)
method_table 70 4 1 3 50 flags &
method_table 100 3 1 3 50 flags &
method_table 70 3 1 4 50 flags &
wait
