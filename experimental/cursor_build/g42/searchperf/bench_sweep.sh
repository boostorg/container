#!/bin/bash
# Group-20 search rows, HEAD vs refactored, over several code layouts.
# Reports median ns/elem for the seg and nsg columns so a single build's
# layout luck cannot masquerade as a real effect.
set -u
BR=/mnt/d/Data/LocalGit/boost
E=$BR/libs/container/experimental
SH=/tmp/shadow/boost/container/experimental
mkdir -p "$SH"
cd "$BR/libs/container"
git show HEAD:include/boost/container/experimental/segmented_search.hpp > "$SH/segmented_search.hpp" || exit 1

cd "$E"
: > /tmp/rows
for cc in g++-16 clang++-22; do
   for variant in HEAD REFACTORED; do
      if [ "$variant" = HEAD ]; then INC="-I/tmp/shadow -I$BR"; else INC="-I$BR"; fi
      for a in 16 32 64 128; do
         out=/tmp/bs.elf
         $cc -std=c++20 -O3 -DNDEBUG -DBENCH_ON \
             -DBOOST_CONTAINER_BENCH_SEGMENTED_GROUP=20 \
             -falign-functions=$a -falign-loops=$a \
             $INC bench_segmented_algos.cpp -o "$out" 2>/dev/null || continue
         taskset -c 3 "$out" 2>/dev/null \
            | awk -v cc="$cc" -v v="$variant" '/^search\(/ {print cc, v, $1, $5, $7}' >> /tmp/rows
      done
   done
done

awk '
  { key = $1 " " $3; seg[key " " $2] = seg[key " " $2] " " $4
                     nsg[key " " $2] = nsg[key " " $2] " " $5
    seen[key] = 1 }
  function med(s) { m = split(s, v, " ")
     for (x=1; x<m; x++) for (y=x+1; y<=m; y++) if (v[y]+0 < v[x]+0) {t=v[x];v[x]=v[y];v[y]=t}
     return (m % 2) ? v[(m+1)/2] : (v[m/2]+v[m/2+1])/2 }
  END {
     printf "%-14s %-14s %-18s %-18s %s\n", "compiler", "row", "seg HEAD->REF", "nsg HEAD->REF", "nsg delta"
     for (k in seen) {
        sh = med(seg[k " HEAD"]); sr = med(seg[k " REFACTORED"])
        nh = med(nsg[k " HEAD"]); nr = med(nsg[k " REFACTORED"])
        split(k, p, " ")
        printf "%-14s %-14s %6.3f -> %-8.3f %6.3f -> %-8.3f %+6.1f%%\n",
               p[1], p[2], sh, sr, nh, nr, (nr/nh - 1) * 100
     }
  }' /tmp/rows | sort
