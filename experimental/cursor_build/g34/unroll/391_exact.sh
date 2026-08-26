#!/bin/bash
set -u
BOOSTROOT=/mnt/d/Data/LocalGit/boost
EXP=$BOOSTROOT/libs/container/experimental
G34=$EXP/cursor_build/g34/unroll
SNAP=$G34/snap
W=/tmp/w391
mkdir -p $W
cd $W

echo "=== header identity: boost-root staging vs libs/include vs snapshot ==="
DIFFCOUNT=0
for f in $BOOSTROOT/libs/container/include/boost/container/experimental/*.hpp; do
  b=$(basename $f)
  a=$(md5sum $f | cut -d' ' -f1)
  c=$(md5sum $BOOSTROOT/boost/container/experimental/$b 2>/dev/null | cut -d' ' -f1)
  d=$(md5sum $SNAP/boost/container/experimental/$b 2>/dev/null | cut -d' ' -f1)
  if [ "$a" != "$c" ] || [ "$a" != "$d" ]; then
    echo "MISMATCH $b libinc=$a boostroot=$c snap=$d"
    DIFFCOUNT=$((DIFFCOUNT+1))
  fi
done
echo "header mismatches: $DIFFCOUNT"

FLAGS="-std=c++20 -O3 -DNDEBUG -DBENCH_ON -DBOOST_CONTAINER_BENCH_SEGMENTED_GROUP=25 -falign-functions=64 -falign-loops=64"

echo
echo "############ EXACT USER COMMAND (from experimental dir) ############"
cd $EXP
echo "--- g++-16 build ---"
/usr/bin/time -f "build %e s" g++-16 $FLAGS -I../../.. bench_segmented_algos.cpp -o $W/gcc_exact.elf 2>&1 | tail -3
echo "--- clang++-22 build ---"
/usr/bin/time -f "build %e s" clang++-22 $FLAGS -I../../.. bench_segmented_algos.cpp -o $W/clang_exact.elf 2>&1 | tail -3

echo
echo "############ SNAPSHOT BUILD (snap include first) ############"
cd $G34
echo "--- g++-16 snap build ---"
/usr/bin/time -f "build %e s" g++-16 $FLAGS -I$SNAP -I$BOOSTROOT bench_segmented_algos.cpp -o $W/gcc_snap.elf 2>&1 | tail -3
echo "--- clang++-22 snap build ---"
/usr/bin/time -f "build %e s" clang++-22 $FLAGS -I$SNAP -I$BOOSTROOT bench_segmented_algos.cpp -o $W/clang_snap.elf 2>&1 | tail -3

echo
echo "=== binary identity exact vs snap ==="
md5sum $W/gcc_exact.elf $W/gcc_snap.elf $W/clang_exact.elf $W/clang_snap.elf

echo
echo "################## EXACT COMMAND OUTPUT: g++-16 ##################"
cd $EXP && $W/gcc_exact.elf
echo
echo "################## EXACT COMMAND OUTPUT: clang++-22 ##################"
cd $EXP && $W/clang_exact.elf

cp $W/*.elf $G34/ 2>/dev/null
echo "DONE"
