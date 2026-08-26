#!/bin/bash
# A/B the real group-20 bench row for search: HEAD's segmented_search.hpp vs the
# refactored one.  HEAD's copy is placed in a shadow include dir that precedes
# $BR on the search path, so the working tree is never modified.
set -u
BR=/mnt/d/Data/LocalGit/boost
E=$BR/libs/container/experimental
SH=/tmp/shadow/boost/container/experimental
mkdir -p "$SH"

cd "$BR/libs/container"
git show HEAD:include/boost/container/experimental/segmented_search.hpp > "$SH/segmented_search.hpp" || exit 1
echo "shadow header: $(wc -l < "$SH/segmented_search.hpp") lines (HEAD)"
echo "working header: $(wc -l < "$BR/boost/container/experimental/segmented_search.hpp") lines (refactored)"
echo

cd "$E"
for cc in g++-16 clang++-22; do
   for variant in HEAD REFACTORED; do
      if [ "$variant" = HEAD ]; then INC="-I/tmp/shadow -I$BR"; else INC="-I$BR"; fi
      out=/tmp/b.$cc.$variant.elf
      if ! $cc -std=c++20 -O3 -DNDEBUG -DBENCH_ON \
             -DBOOST_CONTAINER_BENCH_SEGMENTED_GROUP=20 -falign-functions=64 \
             $INC bench_segmented_algos.cpp -o "$out" 2>/tmp/b.$cc.$variant.log; then
         echo "$cc $variant BUILDFAIL"; grep -m1 'error:' /tmp/b.$cc.$variant.log; continue
      fi
      echo "---------------- $cc / $variant ----------------"
      taskset -c 3 "$out" 2>/dev/null | grep -E 'search|mismatch_2r|geomean|algo'
   done
done
