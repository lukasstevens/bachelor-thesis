#!/usr/bin/env bash
touch_with_trie_column() {
    rm $1
    touch $1
    for ((n=1; n <= $2; ++n)); do
        echo $n >> $1
    done
}

add_header() {
    local temp=$(mktemp)
    printf "$2\n" | cat - $1 > ${temp} && mv ${temp} $1
}

node_count_table() {
    local data_file="k$2i$3div$4.dat"

    touch_with_trie_column ${data_file} $5

    local header='t'
    local -n array=$1
    local temp=$(mktemp)
    for n in "${array[@]}"; do
        local -a flags=(-n $n -k $2 -i $3/$4 -t $5 $6)
        echo "flags: ${flags[@]}"
        (cd $CODE_DIR; craftr build cli["${flags[@]}"]) | tail -n +2 | paste ${data_file} - > ${temp} && mv ${temp} ${data_file}
        header+='\t'
        header+=$n
    done
    add_header ${data_file} ${header}
}

kparts_table() {
    local data_file="n$2i$3div$4.dat"

    touch_with_trie_column ${data_file} $5

    local header='t'
    local -n array=$1
    local temp=$(mktemp)
    for k in "${array[@]}"; do
        local -a flags=(-n $2 -k $k -i $3/$4 -t $5 $6)
        echo "flags: ${flags[@]}"
        (cd $CODE_DIR; craftr build cli["${flags[@]}"]) | tail -n +2 | paste ${data_file} - > ${temp} && mv ${temp} ${data_file}
        header+='\t'
        header+=$k
    done
    add_header ${data_file} ${header}
}

imbalance_table() {
    local data_file="n$2k$3.dat"

    touch_with_trie_column ${data_file} $4

    local header='t'
    local -n array=$1
    local temp=$(mktemp)
    for i in "${array[@]}"; do
        local -a flags=(-n $2 -k $3 -i $i -t $4 $5)
        echo "flags: ${flags[@]}"
        (cd $CODE_DIR; craftr build cli["${flags[@]}"]) | tail -n +2 | paste ${data_file} - > ${temp} && mv ${temp} ${data_file}
        header+='\t'
        header+=$i
    done
    add_header ${data_file} ${header}
}

max_degree_table() {
    local data_file="n$5k$6i$7div$8.dat"

    touch_with_trie_column ${data_file} $9

    local header='t'
    local -n min=$2
    local -n max=$4
    local temp=$(mktemp)
    for ((i=0; i<${#min[@]}; ++i)); do
        local -a flags=(-n $5 -k $6 -i "$7/$8" -t $9 "$1" ${min[$i]} "$3" ${max[$i]} ${10})
        echo "flags: ${flags[@]}"
        (cd $CODE_DIR; craftr build cli["${flags[@]}"]) | tail -n +2 | paste ${data_file} - > ${temp} && mv ${temp} ${data_file}
        header+='\t'
        header+=${max[$i]}
    done
    add_header ${data_file} ${header}
}

divide_columns_by_column() {
    local -n columns=$2
    local column_cnt=$(awk '{print NF; exit}' $1)
    local command=''
    for ((i=1; i<="${column_cnt}"; ++i)); do
        if echo "${columns[@]}" | grep -q -w "$i"; then 
            command+=$(printf " $%s / $%s," "$3" "$i")
        else 
            command+=$(printf " $%s," "$i")
        fi
    done
    local command_trimmed=$(echo "${command%?}")
    local awk_command=$(printf "{ print %s }" "${command_trimmed}")
    awk "${awk_command}" OFS='\t' $1
}

method_table() {
    local -a methods=(-m Tree_Partition -m METIS_Recursive -m METIS_Kway -m KaFFPa)
    local -a other_methods_cols=(2 3 4)
    local -a params=(-n $1 -k $2 -i "$3/$4" -t $5 -o cut_cost)
    local -n add_flags=$6

    local data_file="n$1k$2i$3div$4.dat"
    local temp_one=$(mktemp)
    local temp_two=$(mktemp)
    touch_with_trie_column "${data_file}" $5

    (cd ${CODE_DIR} && craftr build cli["${params[@]}" "${methods[@]}" "${add_flags[@]}"]) | tail -n +2  > "${temp_one}"

    divide_columns_by_column "${temp_one}" other_methods_cols 1 > "${temp_two}" && mv "${temp_two}" "${temp_one}"
    paste "${data_file}" "${temp_one}" > "${temp_two}" && mv "${temp_two}" "${data_file}"
    
    local header='t'
    for ((i=1; i<=${#methods[@]}; i+=2)); do
        header+="\t ${methods[$i]}"
    done
    add_header "${data_file}" "${header}"
}

