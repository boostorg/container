#!/bin/bash
# Builds bench_segmented_algos binaries for groups 10 and 25, variants
# base/T/F, compilers g++-16/clang++-22.  Binaries persist under out/bin.
set -u
BR=/mnt/d/Data/LocalGit/boost
G=$BR/libs/container/experimental/cursor_build/g28/pattern
EX=$BR/libs/container/experimental
BIN=$G/out/bin
mkdir -p $BIN

build_one() {
   local CC=$1 V=$2 GRP=$3
   local INC ALIGN=""
   if [ $V = base ]; then INC="-I$BR"; else INC="-I$G/shadow_$V -I$BR"; fi
   [ "${CC#clang}" != "$CC" ] && ALIGN="-falign-functions=64 -falign-loops=64"
   $CC -std=c++20 -O2 -DNDEBUG -DBENCH_ON -DBOOST_CONTAINER_BENCH_SEGMENTED_GROUP=$GRP \
      $ALIGN $INC -I$EX $EX/bench_segmented_algos.cpp -o $BIN/bench.$CC.$V.g$GRP \
      2>$BIN/bench.$CC.$V.g$GRP.log \
      && echo "OK   $CC $V g$GRP" || { echo "FAIL $CC $V g$GRP"; head -10 $BIN/bench.$CC.$V.g$GRP.log; }
}

njobs=0
for GRP in 10 25; do
   for CC in g++-16 clang++-22; do
      for V in base T F; do
         build_one $CC $V $GRP &
         njobs=$((njobs+1))
         [ $((njobs % 4)) -eq 0 ] && wait
      done
   done
done
wait
ls -l $BIN | grep -v '\.log'
