#!/bin/bash
set -u
BR=/mnt/d/Data/LocalGit/boost
cd $BR/libs/container/experimental || exit 1
for CXX in g++-16 clang++-22; do
   echo "==================== $CXX ===================="
   $CXX -std=c++20 -O3 -I$BR -DNDEBUG -DBENCH_ON -DBOOST_CONTAINER_BENCH_SEGMENTED_GROUP=20 \
        bench_segmented_algos.cpp -o /tmp/g20.$CXX.elf 2>/tmp/g20.$CXX.log || { head -20 /tmp/g20.$CXX.log; continue; }
   taskset -c 4 /tmp/g20.$CXX.elf | grep -E "equal|mismatch|geomean|algo|deque|Elements"
done
