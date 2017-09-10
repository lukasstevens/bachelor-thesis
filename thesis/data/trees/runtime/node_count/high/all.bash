#!/usr/bin/env bash
declare -a node_counts=(100 200 400 800)
run() {
    for n in "${node_counts[@]}"; do
        ../../../../../../code/cpp/build/bachelorthesis-1.0.0/cli -m Tree_Partition -g $1 -n $n -o time -k 2 -i 1/2 $2 2> /dev/null | tr "\n" "\t"
    done
}

out=high_node_count.dat
(printf 'tn'; printf '\t%s' "${node_counts[@]}"; printf '\n') > "$out"
run "tree_rand_attach" "" | awk '{print "rand\t" $0}' >>  "$out"
run "tree_pref_attach" "" | awk '{print "pref\t" $0}' >>  "$out"
run "tree_fat" "--min_child=2 --max_child=11" | awk '{print "fat " $0}' >>  "$out"
run "tree_fat" "--min_child=2 --max_child=3" | awk '{print "bnary " $0}' >>  "$out"

