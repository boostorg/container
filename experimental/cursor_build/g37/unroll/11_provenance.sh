#!/bin/bash
set -u
W=/mnt/d/Data/LocalGit/boost/libs/container/experimental/cursor_build/g37/unroll
g++-16 -std=c++20 -O0 -fsyntax-only -H -DNDEBUG -DBENCH_ON \
   -DBOOST_CONTAINER_BENCH_SEGMENTED_GROUP=25 \
   -I$W/exp/p2_all4 -I$W/snap -I/mnt/d/Data/LocalGit/boost \
   $W/bench_g37.cpp 2>&1 | grep -E 'boost/container/' | tr -d '. ' | sort -u > /tmp/hdrs.txt

echo "total boost/container headers pulled: $(wc -l < /tmp/hdrs.txt)"
echo "from LIVE libs/container/include : $(grep -c 'libs/container/include' /tmp/hdrs.txt)"
echo "from snap                        : $(grep -c 'g37/unroll/snap/' /tmp/hdrs.txt)"
echo "from exp overlay                 : $(grep -c 'g37/unroll/exp/' /tmp/hdrs.txt)"
echo "from staged boost root           : $(grep -cE '^/mnt/d/Data/LocalGit/boost/boost/' /tmp/hdrs.txt)"
echo "--- overlay files actually used ---"
grep 'g37/unroll/exp/' /tmp/hdrs.txt
echo "--- any live-tree leak ---"
grep 'libs/container/include' /tmp/hdrs.txt || echo "(none)"
