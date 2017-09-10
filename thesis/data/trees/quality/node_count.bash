#!/usr/bin/env bash
(cd ../../../code/cpp && craftr build test[--gtest_also_run_disabled_tests --gtest_filter=DISABLED_Trees.NodeCount])
mv ../../../../code/cpp/build/*.dat node_count
