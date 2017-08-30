#!/usr/bin/env bash
node_count_table() {
    local data_file="k$2i$3div$4.dat"

    rm ${data_file}
    touch ${data_file}
    for ((n=1; n <= $5; ++n)) do
        echo $n >> ${data_file}
    done
    local header='t'

    local -n array=$1
    local temp=$(mktemp)
    for n in "${array[@]}"; do
        local -a flags=(-n $n -k $2 -i $3/$4 -t $5 $6)
        echo "flags: ${flags[@]}"
        (cd $CODE_DIR; craftr build cli["${flags[@]}"]) | sed '/[\d\/\d].*$/d' | paste ${data_file} - > ${temp} && mv ${temp} ${data_file}
        header+='\t'
        header+=$n
    done
    printf "${header}\n" | cat - ${data_file} > ${temp} && mv ${temp} ${data_file}
}

kparts_table() {
    local data_file="n$2i$3div$4.dat"

    rm ${data_file}
    touch ${data_file}
    for ((n=1; n <= $5; ++n)) do
        echo $n >> ${data_file}
    done
    local header='t'

    local -n array=$1
    local temp=$(mktemp)
    for k in "${array[@]}"; do
        local -a flags=(-n $2 -k $k -i $3/$4 -t $5 $6)
        echo "flags: ${flags[@]}"
        (cd $CODE_DIR; craftr build cli["${flags[@]}"]) | sed '/[\d\/\d].*$/d' | paste ${data_file} - > ${temp} && mv ${temp} ${data_file}
        header+='\t'
        header+=$k
    done
    printf "${header}\n" | cat - ${data_file} > ${temp} && mv ${temp} ${data_file}
}

imbalance_table() {
    local data_file="n$2k$3.dat"

    rm ${data_file}
    touch ${data_file}
    for ((n=1; n <= $4; ++n)) do
        echo $n >> ${data_file}
    done
    local header='t'

    local -n array=$1
    local temp=$(mktemp)
    for i in "${array[@]}"; do
        local -a flags=(-n $2 -k $3 -i $i -t $4 $5)
        echo "flags: ${flags[@]}"
        (cd $CODE_DIR; craftr build cli["${flags[@]}"]) | sed '/[\d\/\d].*$/d' | paste ${data_file} - > ${temp} && mv ${temp} ${data_file}
        header+='\t'
        header+=$i
    done
    printf "${header}\n" | cat - ${data_file} > ${temp} && mv ${temp} ${data_file}
}
