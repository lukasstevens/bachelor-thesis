#!/usr/bin/env bash
node_count_table() {
    data_file="k$2i$3div$4.dat"

    rm ${data_file}
    touch ${data_file}
    for ((n=1; n <= $5; ++n)) do
        echo $n >> ${data_file}
    done
    header='t'

    declare -n array=$1
    echo ${array[@]}
    for n in "${array[@]}"; do
        (cd ../../../code/cpp; pwd; craftr -q build cli[-g tree_rand_attach -n $n -m Tree_Partition -k $2 -i $3/$4 -o time -t $5]) | sed '/[\d\/\d].*$/d' | paste ${data_file} - > tmp && mv tmp ${data_file}
        header+='\t'
        header+=$n
    done
    printf "${header}\n" | cat - ${data_file} > tmp && mv tmp ${data_file}
}
