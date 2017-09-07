#!/usr/bin/env bash
for i in 200 1000 5000 10000; do
    ../../build/bachelorthesis-1.0.0/cli --output_mod=tree -p contract --contract_to=60 -g graph_pref_attach -n $i -e 10 -t 10 -o graph  | java -jar ../../deps/hierarchical-decomposition/build/libs/hierarchical-decomposition.jar 10 > "contract_pref_attach10n$i.graph" &
    ../../build/bachelorthesis-1.0.0/cli --output_mod=tree -p contract --contract_to=60 -g graph_pref_attach -n $i -e 25 -t 10 -o graph | java -jar ../../deps/hierarchical-decomposition/build/libs/hierarchical-decomposition.jar 10 > "contract_pref_attach25n$i.graph" &
    wait
    ../../build/bachelorthesis-1.0.0/cli --output_mod=tree -p contract --contract_to=60 -g graph_edge_prob -n $i --edge_prob=0.1 -t 10 -o graph  | java -jar ../../deps/hierarchical-decomposition/build/libs/hierarchical-decomposition.jar 10 > "contract_edge_prob1n$i.graph" &
    ../../build/bachelorthesis-1.0.0/cli --output_mod=tree -p contract --contract_to=60 -g graph_edge_prob -n $i --edge_prob=0.3 -t 10 -o graph | java -jar ../../deps/hierarchical-decomposition/build/libs/hierarchical-decomposition.jar 10 > "contract_edge_prob3n$i.graph" &
    wait
done



