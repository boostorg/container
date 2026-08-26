#!/bin/bash
# Same source, several code layouts.  Ratios within one binary are meaningful;
# across binaries absolute times move a lot, so report the median over layouts.
# The aggregation key carries the haystack kind, unlike g42/sweep.sh.
set -u
BR=/mnt/d/Data/LocalGit/boost
CB=$BR/libs/container/bench
H=$BR/libs/container/experimental/cursor_build/g45/ownsearch

for cc in g++-16 clang++-22; do
   echo "================ $cc (median of 6 layouts, ratio vs flat) ================"
   : > /tmp/agg.$cc
   for a in 16 32 64 128 256 512; do
      $cc -std=c++17 -O2 -DNDEBUG -falign-functions=$a -falign-loops=$a \
          -I"$BR" -I"$CB" "$H/010_probe.cpp" -o "/tmp/ow.elf" 2>/dev/null || { echo "  build fail align=$a"; continue; }
      taskset -c 3 /tmp/ow.elf >> /tmp/agg.$cc
   done
   grep -h 'ARM DISAGREEMENT' /tmp/agg.$cc | sort -u
   awk '
     /^ROW / {
        key = $2
        for (i = 1; i <= NF; i++) {
           if ($i ~ /^mm\/flat=/)    { split($i, a, "="); mm[key]    = mm[key]    " " a[2] }
           if ($i ~ /^own\/flat=/)   { split($i, a, "="); own[key]   = own[key]   " " a[2] }
           if ($i ~ /^ownc\/flat=/)  { split($i, a, "="); ownc[key]  = ownc[key]  " " a[2] }
           if ($i ~ /^owns\/flat=/)  { split($i, a, "="); owns[key]  = owns[key]  " " a[2] }
           if ($i ~ /^ownr\/flat=/)  { split($i, a, "="); ownr[key]  = ownr[key]  " " a[2] }
           if ($i ~ /^ownrc\/flat=/) { split($i, a, "="); ownrc[key] = ownrc[key] " " a[2] }
           if ($i ~ /^lib\/flat=/)   { split($i, a, "="); lib[key]   = lib[key]   " " a[2] }
        }
        if (!(key in order)) { order[key] = ++n; name[n] = key }
     }
     function med(s,   m, v, x, y, t) {
        m = split(s, v, " ")
        if (!m) return 0
        for (x = 1; x < m; x++) for (y = x+1; y <= m; y++)
           if (v[y]+0 < v[x]+0) { t = v[x]; v[x] = v[y]; v[y] = t }
        return (m % 2) ? v[(m+1)/2]+0 : (v[m/2] + v[m/2+1]) / 2
     }
     END {
        printf "  %-26s %7s %7s %7s %7s %7s %7s %7s\n", "config", "mm", "own", "ownc", "owns", "ownr", "ownrc", "lib"
        for (i = 1; i <= n; i++) {
           k = name[i]
           printf "  %-26s %7.2f %7.2f %7.2f %7.2f %7.2f %7.2f %7.2f\n", k,
                  med(mm[k]), med(own[k]), med(ownc[k]), med(owns[k]), med(ownr[k]), med(ownrc[k]), med(lib[k])
        }
     }' /tmp/agg.$cc
done
