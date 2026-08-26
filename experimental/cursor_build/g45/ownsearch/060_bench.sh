#!/bin/bash
# Group-20 search rows: HEAD vs the mismatch-delegating version vs the new one,
# over several code layouts, reported as medians.  Both older variants live in
# shadow include dirs placed ahead of the boost root, so the working tree is
# never touched.
set -u
BR=/mnt/d/Data/LocalGit/boost
E=$BR/libs/container/experimental
H=$E/cursor_build/g45/ownsearch
HEADSH=/tmp/head_shadow/boost/container/experimental
mkdir -p "$HEADSH"
cd "$BR/libs/container"
git show HEAD:include/boost/container/experimental/segmented_search.hpp > "$HEADSH/segmented_search.hpp" || exit 1

cd "$E"
: > /tmp/rows45
for cc in g++-16 clang++-22; do
   for variant in HEAD MM NEW; do
      case $variant in
         HEAD) INC="-I/tmp/head_shadow -I$BR" ;;
         MM)   INC="-I$H/mm_shadow -I$BR" ;;
         NEW)  INC="-I$BR" ;;
      esac
      for a in 16 32 64 128 256 512; do
         $cc -std=c++20 -O3 -DNDEBUG -DBENCH_ON \
             -DBOOST_CONTAINER_BENCH_SEGMENTED_GROUP=20 \
             -falign-functions=$a -falign-loops=$a \
             $INC bench_segmented_algos.cpp -o /tmp/bs45.elf 2>/tmp/bs45.log \
            || { echo "BUILDFAIL $cc $variant align=$a"; sed -n '1,8p' /tmp/bs45.log; continue; }
         taskset -c 3 /tmp/bs45.elf 2>/dev/null \
            | awk -v cc="$cc" -v v="$variant" '/^search\(/ {print cc, v, $1, $5, $7}' >> /tmp/rows45
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
     printf "%-12s %-14s | %-24s | %-24s | %s\n", "compiler", "row",
            "seg  HEAD / MM / NEW", "nsg  HEAD / MM / NEW", "NEW vs HEAD (seg,nsg)"
     for (k in seen) {
        sgh = med(seg[k " HEAD"]); sgm = med(seg[k " MM"]); sgn = med(seg[k " NEW"])
        nsh = med(nsg[k " HEAD"]); nsm = med(nsg[k " MM"]); nsn = med(nsg[k " NEW"])
        split(k, p, " ")
        printf "%-12s %-14s | %6.3f %7.3f %7.3f  | %6.3f %7.3f %7.3f  | %+5.1f%% %+6.1f%%\n",
               p[1], p[2], sgh, sgm, sgn, nsh, nsm, nsn,
               (sgh ? (sgn/sgh - 1) * 100 : 0), (nsh ? (nsn/nsh - 1) * 100 : 0)
     }
  }' /tmp/rows45 | sort
