#!/usr/bin/env bash
for ((i=70; i<=100; i+=10)); do
    #../../build/bachelorthesis-1.0.0/cli -g graph_edge_prob -n $i --edge_prob=0.1 -t 10 -o graph | java -jar ../../deps/hierarchical-decomposition/build/libs/hierarchical-decomposition.jar 10 > "edge_prob1n$i.graph" &
    #../../build/bachelorthesis-1.0.0/cli -g graph_edge_prob -n $i --edge_prob=0.3 -t 10 -o graph | java -jar ../../deps/hierarchical-decomposition/build/libs/hierarchical-decomposition.jar 10 > "edge_prob3n$i.graph" &
    ../../build/bachelorthesis-1.0.0/cli -g graph_pref_attach -n $i -e 5 -t 10 -o graph | java -jar ../../deps/hierarchical-decomposition/build/libs/hierarchical-decomposition.jar 10 > "pref_attach5n$i.graph" &
    ../../build/bachelorthesis-1.0.0/cli -g graph_pref_attach -n $i -e 10 -t 10 -o graph | java -jar ../../deps/hierarchical-decomposition/build/libs/hierarchical-decomposition.jar 10 > "pref_attach10n$i.graph" &
    wait
done


