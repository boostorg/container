#!/bin/bash
set -u
BOOSTROOT=/mnt/d/Data/LocalGit/boost
EXP=$BOOSTROOT/libs/container/experimental
G34=$EXP/cursor_build/g34/unroll
SNAP=$G34/snap
SNAPON=$G34/snap_on
OUT=$G34/runs
mkdir -p $OUT

# Build an "unroll ON" twin of the snapshot: only the traits header differs.
rm -rf $SNAPON
cp -r $SNAP $SNAPON
T=$SNAPON/boost/container/experimental/segmented_iterator_traits.hpp
sed -i 's|^#define BOOST_CONTAINER_SEGMENTED_DISABLE_PRAGMA_UNROLL|//#define BOOST_CONTAINER_SEGMENTED_DISABLE_PRAGMA_UNROLL|' $T
sed -i 's|^//#define BOOST_CONTAINER_SEGMENTED_ENABLE_PRAGMA_UNROLL|#define BOOST_CONTAINER_SEGMENTED_ENABLE_PRAGMA_UNROLL|' $T
echo "=== snap_on traits switch ==="
sed -n '396,398p' $T
echo "=== diff snap vs snap_on (should be traits only) ==="
diff -rq $SNAP $SNAPON

FLAGS="-std=c++20 -O3 -DNDEBUG -DBENCH_ON -DBOOST_CONTAINER_BENCH_SEGMENTED_GROUP=25 -falign-functions=64 -falign-loops=64"
cd $G34/exp

echo "=== builds ==="
g++-16     $FLAGS -I$SNAP   -I$BOOSTROOT bench_segmented_algos.cpp -o $G34/gcc_off.elf   2>&1|head -5
g++-16     $FLAGS -I$SNAPON -I$BOOSTROOT bench_segmented_algos.cpp -o $G34/gcc_on.elf    2>&1|head -5
clang++-22 $FLAGS -I$SNAP   -I$BOOSTROOT bench_segmented_algos.cpp -o $G34/clang_off.elf 2>&1|head -5
clang++-22 $FLAGS -I$SNAPON -I$BOOSTROOT bench_segmented_algos.cpp -o $G34/clang_on.elf  2>&1|head -5
ls -l $G34/gcc_off.elf $G34/gcc_on.elf $G34/clang_off.elf $G34/clang_on.elf
echo "=== md5: does unroll flag change the binary? ==="
md5sum $G34/gcc_off.elf $G34/gcc_on.elf $G34/clang_off.elf $G34/clang_on.elf

for cfg in gcc_off gcc_on clang_off clang_on; do
  echo "=== running $cfg x5 ==="
  for i in 1 2 3 4 5; do
    taskset -c 3 $G34/$cfg.elf > $OUT/$cfg.$i.txt 2>&1
  done
done

# median of 5 for seg/std/nsg ns per row, recompute ratios from medians
med() { sort -g | awk '{a[NR]=$1} END{print a[int((NR+1)/2)]}'; }

for cfg in gcc_off gcc_on clang_off clang_on; do
  echo
  echo "############ MEDIAN OF 5 PINNED RUNS: $cfg ############"
  printf '%-28s%12s%12s%12s%11s%11s%11s\n' '< algo >' '<nsg/seg>' '<std/seg>' '<std/nsg>' '<seg ns>' '<std ns>' '<nsg ns>'
  grep -h '^[a-z]' $OUT/$cfg.1.txt | grep -E '^(copy|remove|swap|transform)' | cut -c1-28 | while IFS= read -r lab; do
    key=$(echo "$lab" | sed 's/ *$//')
    segv=$(for i in 1 2 3 4 5; do grep -F "$key " $OUT/$cfg.$i.txt | head -1 | awk '{print $(NF-2)}'; done | med)
    stdv=$(for i in 1 2 3 4 5; do grep -F "$key " $OUT/$cfg.$i.txt | head -1 | awk '{print $(NF-1)}'; done | med)
    nsgv=$(for i in 1 2 3 4 5; do grep -F "$key " $OUT/$cfg.$i.txt | head -1 | awk '{print $NF}'; done | med)
    awk -v l="$key" -v s="$segv" -v t="$stdv" -v n="$nsgv" 'BEGIN{
      printf "%-28s%12.2f%12.2f%12.2f%11.3f%11.3f%11.3f\n", l, n/s, t/s, t/n, s, t, n }'
  done
done
echo DONE
