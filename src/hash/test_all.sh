#!/bin/bash

TC=100MB.txt

./gnu/hsearch $TC
./gnu/unordered_map $TC
./google/sparsehash-master/simple_test $TC

