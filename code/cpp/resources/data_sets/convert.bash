#!/usr/bin/env bash
convert() {
    cat "$1.txt" | ../../../build/bachelorthesis-1.0.0/snapconvert > "$1.graph"
    cat "$1.graph" | ../../../build/bachelorthesis-1.0.0/cli -p contract --contract_to="$2" --file=- --output_mod=tree -o graph > "$1_contracted.graph"
    cat "$1_contracted.graph" | java -jar ../../../deps/hierarchical-decomposition/build/libs/hierarchical-decomposition.jar > "$1_decomposed.graph"
    rm Log.*
}
