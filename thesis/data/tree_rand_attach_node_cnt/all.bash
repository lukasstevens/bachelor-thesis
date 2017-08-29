#!/usr/bin/env bash
source ../node_count_table.bash

node_counts=(70 90 110 130)
node_count_table node_counts 2 1 3 20
node_count_table node_counts 3 1 3 20
node_count_table node_counts 2 1 4 20
