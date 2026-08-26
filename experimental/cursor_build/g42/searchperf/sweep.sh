#!/bin/bash
# Same source, several code layouts.  Ratios within one binary are meaningful;
# across binaries they move a lot, so report the median over layouts.
set -u
BR=/mnt/d/Data/LocalGit/boost
CB=$BR/libs/container/bench
H=$BR/libs/container/experimental/cursor_build/g42/searchperf

for cc in g++-16 clang++-22; do
   echo "================ $cc (median of 6 layouts) ================"
   : > /tmp/agg.$cc
   for a in 16 32 64 128 256 512; do
      $cc -std=c++17 -O2 -DNDEBUG -falign-functions=$a -falign-loops=$a \
          -I"$BR" -I"$CB" "$H/probe2.cpp" -o "/tmp/sw.elf" 2>/dev/null || continue
      taskset -c 3 /tmp/sw.elf >> /tmp/agg.$cc
   done
   awk '
     /new\/old=/ {
        key = $1 " " $2 " " $3
        for (i = 1; i <= NF; i++) {
           if ($i ~ /^new\/old=/) { split($i, a, "="); r[key] = r[key] " " a[2] }
        }
        order[key] = order[key] ? order[key] : ++n
        name[order[key]] = key
     }
     END {
        printf "  %-28s %8s %8s %8s   %s\n", "config", "median", "min", "max", "(mismatch/flat)"
        for (i = 1; i <= n; i++) {
           k = name[i]; m = split(r[k], v, " ")
           for (x = 1; x < m; x++) for (y = x+1; y <= m; y++)
              if (v[y]+0 < v[x]+0) { t=v[x]; v[x]=v[y]; v[y]=t }
           med = (m % 2) ? v[(m+1)/2] : (v[m/2] + v[m/2+1]) / 2
           verdict = (med+0 > 1.08) ? "SLOWER" : ((med+0 < 0.92) ? "faster" : "~equal")
           printf "  %-28s %8.2f %8.2f %8.2f   %s\n", k, med, v[1], v[m], verdict
        }
     }' /tmp/agg.$cc
done
