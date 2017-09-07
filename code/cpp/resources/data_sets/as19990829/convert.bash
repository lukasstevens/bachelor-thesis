#!/usr/bin/env bash
cat as19990829.txt | ../../../build/bachelorthesis-1.0.0/snapconvert > as19990829.graph
cp as19990829.graph as19990829_contracted.graph
cat as19990829_contracted.graph | java -jar ../../../deps/hierarchical-decomposition/build/libs/hierarchical-decomposition.jar > as19990829_decomposed.graph
rm Log.txt
