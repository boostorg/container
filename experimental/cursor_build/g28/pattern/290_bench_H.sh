#!/bin/bash
# Build + run the H bench binaries (groups 10 and 25, both compilers).
set -u
BR=/mnt/d/Data/LocalGit/boost
G=$BR/libs/container/experimental/cursor_build/g28/pattern
EX=$BR/libs/container/experimental
BIN=$G/out/bin
RUNS=$G/out/bench
mkdir -p $BIN $RUNS

for GRP in 10 25; do
   for CC in g++-16 clang++-22; do
      ALIGN=""
      [ "${CC#clang}" != "$CC" ] && ALIGN="-falign-functions=64 -falign-loops=64"
      $CC -std=c++20 -O2 -DNDEBUG -DBENCH_ON -DBOOST_CONTAINER_BENCH_SEGMENTED_GROUP=$GRP \
         $ALIGN -I$G/shadow_H -I$BR -I$EX $EX/bench_segmented_algos.cpp \
         -o $BIN/bench.$CC.H.g$GRP 2>$BIN/bench.$CC.H.g$GRP.log &
   done
done
wait
echo "builds done"

if sudo -n true 2>/dev/null; then PIN="sudo -n chrt -f 90 taskset -c 3"; else PIN="nice -n -5 taskset -c 3"; fi
echo "pin: $PIN"
for GRP in 10 25; do
   for CC in g++-16 clang++-22; do
      for i in 1 2 3 4 5; do
         $PIN $BIN/bench.$CC.H.g$GRP > $RUNS/bench.$CC.H.g$GRP.run$i.txt 2>&1
         echo "done $CC H g$GRP run$i"
      done
   done
done
