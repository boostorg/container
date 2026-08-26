#!/bin/bash
set -u
BR=/mnt/d/Data/LocalGit/boost
EX=$BR/libs/container/experimental
O=/tmp/g21b30
mkdir -p $O
cd $EX || exit 1

for CXX in g++-16 clang++-22; do
   echo "######################## $CXX  group 30 ########################"
   $CXX -std=c++20 -O3 -I$BR -DNDEBUG -DBENCH_ON \
        -DBOOST_CONTAINER_BENCH_SEGMENTED_GROUP=30 \
        bench_segmented_algos.cpp -o $O/b.$CXX.elf 2>$O/b.$CXX.log \
      || { echo BUILD FAIL; grep -m5 'error:' $O/b.$CXX.log; continue; }
   # high priority, pinned, to keep the numbers comparable with earlier runs
   sudo -n chrt -f 90 taskset -c 3 $O/b.$CXX.elf 2>/dev/null \
      || nice -n -5 taskset -c 3 $O/b.$CXX.elf 2>/dev/null \
      || taskset -c 3 $O/b.$CXX.elf
done
