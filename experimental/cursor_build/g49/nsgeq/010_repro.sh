#!/bin/bash
set -u
E=/mnt/d/Data/LocalGit/boost/libs/container/experimental
cd "$E"
clang++-22 -std=c++20 -O3 -I../../.. -DNDEBUG -DBENCH_ON -DBOOST_CONTAINER_BENCH_SEGMENTED_GROUP=20 \
   -falign-functions=64 -falign-loops=64 bench_segmented_algos.cpp -o /tmp/g49_a.elf
taskset -c 3 /tmp/g49_a.elf
