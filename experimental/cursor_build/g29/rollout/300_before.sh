#!/bin/bash
# Baseline (pre-H) build: all segmented test binaries (code size) and the
# benchmark binaries for groups 10/15/20/25/30, both compilers.
# Binaries persist under g29/rollout/bin_before (WSL /tmp is wiped).
set -u
BR=/mnt/d/Data/LocalGit/boost
EX=$BR/libs/container/experimental
G=$BR/libs/container/experimental/cursor_build/g29/rollout
O=$G/bin_before
mkdir -p $O
fails=0

# Ensure tree is pristine (headers unmodified)
cd $BR/libs/container
if ! git diff --quiet -- include; then
   echo "ERROR: include/ has local modifications; baseline would be wrong"
   exit 2
fi

ALIGN_CLANG="-falign-functions=64 -falign-loops=64"

echo "=== test binaries (code size baseline), -O2 -DNDEBUG -std=c++20 ==="
for CC in g++-16 clang++-22; do
   for T in $EX/segmented_*_test.cpp; do
      B=$(basename $T .cpp)
      if ! $CC -std=c++20 -O2 -DNDEBUG -I$BR -I$EX $T -o $O/$B.$CC 2>$O/$B.$CC.log; then
         echo "BUILD FAIL $B $CC"; fails=1
      fi
   done
   echo "  $CC done"
done

echo "=== bench binaries, groups 10 15 20 25 30 ==="
for CC in g++-16 clang++-22; do
   EXTRA=""
   [ "$CC" = "clang++-22" ] && EXTRA="$ALIGN_CLANG"
   for GRP in 10 15 20 25 30; do
      if ! $CC -std=c++20 -O2 -DNDEBUG -DBENCH_ON -DBOOST_CONTAINER_BENCH_SEGMENTED_GROUP=$GRP \
            $EXTRA -I$BR -I$EX $EX/bench_segmented_algos.cpp -o $O/bench.$GRP.$CC 2>$O/bench.$GRP.$CC.log; then
         echo "BENCH BUILD FAIL group $GRP $CC"; fails=1
      fi
   done
   echo "  $CC bench done"
done

echo "=== .text size baseline ==="
for CC in g++-16 clang++-22; do
   total=0
   for B in $O/segmented_*_test.$CC; do
      t=$(size -A "$B" | awk '$1==".text"{print $2}')
      total=$((total + t))
   done
   echo "$CC total .text: $total"
done > $O/text_sizes.txt
cat $O/text_sizes.txt

[ $fails -eq 0 ] && echo "300: ALL OK" || echo "300: FAILURES PRESENT"
