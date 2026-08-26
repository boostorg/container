#!/bin/bash
# Is the group-20 nsg regression the clamp or the bespoke verify?
#   HEAD    - pre-refactor header
#   NOCLAMP - new header with the clamped dispatch disabled
#   NEW     - new header as committed
set -u
BR=/mnt/d/Data/LocalGit/boost
E=$BR/libs/container/experimental
H=$E/cursor_build/g45/ownsearch
HEADSH=/tmp/head_shadow/boost/container/experimental
mkdir -p "$HEADSH"
cd "$BR/libs/container"
git show HEAD:include/boost/container/experimental/segmented_search.hpp > "$HEADSH/segmented_search.hpp" || exit 1

cd "$E"
: > /tmp/rows47
for cc in g++-16 clang++-22; do
   for variant in HEAD NOCLAMP NEW; do
      case $variant in
         HEAD)    INC="-I/tmp/head_shadow -I$BR" ;;
         NOCLAMP) INC="-I$H/noclamp_shadow -I$BR" ;;
         NEW)     INC="-I$BR" ;;
      esac
      for a in 16 32 64 128; do
         $cc -std=c++20 -O3 -DNDEBUG -DBENCH_ON \
             -DBOOST_CONTAINER_BENCH_SEGMENTED_GROUP=20 \
             -falign-functions=$a -falign-loops=$a \
             $INC bench_segmented_algos.cpp -o /tmp/bs47.elf 2>/tmp/bs47.log \
            || { echo "BUILDFAIL $cc $variant align=$a"; sed -n '1,8p' /tmp/bs47.log; continue; }
         taskset -c 3 /tmp/bs47.elf 2>/dev/null \
            | awk -v cc="$cc" -v v="$variant" '/^search\(/ {print cc, v, $1, $5, $7}' >> /tmp/rows47
      done
   done
done

awk '
  { key = $1 " " $3
    seg[key " " $2] = seg[key " " $2] " " $4
    nsg[key " " $2] = nsg[key " " $2] " " $5
    seen[key] = 1 }
  function med(s,   m, v, x, y, t) {
     m = split(s, v, " ")
     if (!m) return 0
     for (x = 1; x < m; x++) for (y = x+1; y <= m; y++)
        if (v[y]+0 < v[x]+0) { t = v[x]; v[x] = v[y]; v[y] = t }
     return (m % 2) ? v[(m+1)/2]+0 : (v[m/2] + v[m/2+1]) / 2 }
  END {
     printf "%-12s %-14s | %-23s | %-23s\n", "compiler", "row",
            "seg HEAD/NOCLAMP/NEW", "nsg HEAD/NOCLAMP/NEW"
     for (k in seen) {
        a1 = med(seg[k " HEAD"]); a2 = med(seg[k " NOCLAMP"]); a3 = med(seg[k " NEW"])
        b1 = med(nsg[k " HEAD"]); b2 = med(nsg[k " NOCLAMP"]); b3 = med(nsg[k " NEW"])
        split(k, p, " ")
        printf "%-12s %-14s | %6.3f %7.3f %7.3f | %6.3f %7.3f %7.3f\n",
               p[1], p[2], a1, a2, a3, b1, b2, b3
     }
  }' /tmp/rows47 | sort
