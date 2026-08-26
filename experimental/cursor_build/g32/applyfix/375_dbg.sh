#!/bin/bash
set -u
BR=/mnt/d/Data/LocalGit/boost
EX=$BR/libs/container/experimental
O=/tmp/g32dbg
mkdir -p $O
echo "arg1=[${1:-}] arg2=[${2:-}]"
G=25
echo "building group $G with g++-16 ..."
time g++-16 -std=c++20 -O3 -I$BR -DNDEBUG -DBENCH_ON \
   -DBOOST_CONTAINER_BENCH_SEGMENTED_GROUP=$G \
   -falign-functions=64 -falign-loops=64 \
   $EX/bench_segmented_algos.cpp -o $O/b 2> $O/b.log
echo "build rc=$?"
head -20 $O/b.log
echo "running ..."
time taskset -c 3 $O/b > $O/out.txt 2>&1
echo "run rc=$?  lines=$(wc -l < $O/out.txt)"
sed -n '1,60p' $O/out.txt
