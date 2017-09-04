#!/usr/bin/env bash
for ((i=70; i<=100; i+=10)); do
    ../../../../build/bachelorthesis-1.0.0/cli -g graph_edge_prob -n $i --edge_prob=0.1 -t 10 -o graph | java -jar ../../../../deps/hierarchical-decomposition/build/libs/hierarchical-decomposition.jar 10 > "edge_prob1n$i.graph" &
done
wait

for ((i=70; i<=100; i+=10)); do
    ../../../../build/bachelorthesis-1.0.0/cli -g graph_edge_prob -n $i --edge_prob=0.3 -t 10 -o graph | java -jar ../../../../deps/hierarchical-decomposition/build/libs/hierarchical-decomposition.jar 10 > "edge_prob3n$i.graph" &
done
wait

for ((i=70; i<=100; i+=10)); do
    ../../../../build/bachelorthesis-1.0.0/cli -g graph_pref_attach -n $i -e "$(($i * $i / 10 ))" -t 10 -o graph | java -jar ../../../../deps/hierarchical-decomposition/build/libs/hierarchical-decomposition.jar 10 > "pref_attach1n$i.graph" &
done
wait

for ((i=70; i<=100; i+=10)); do
    ../../../../build/bachelorthesis-1.0.0/cli -g graph_pref_attach -n $i -e "$((3 * $i * $i / 10 ))" -t 10 -o graph | java -jar ../../../../deps/hierarchical-decomposition/build/libs/hierarchical-decomposition.jar 10 > "pref_attach3n$i.graph" &
done
wait


