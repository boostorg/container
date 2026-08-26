#!/bin/bash
# Three-way group-20 A/B over four layouts:
#   HEAD    - two duplicated dispatches (flat verify when non-segmented)
#   REFAC   - unified dispatch, mismatch verify everywhere
#   SPLIT   - unified dispatch, mismatch verify only for segmented haystacks
set -u
BR=/mnt/d/Data/LocalGit/boost
E=$BR/libs/container/experimental
H=$E/cursor_build/g42/searchperf
HDR=boost/container/experimental/segmented_search.hpp

mkdir -p /tmp/shadow/boost/container/experimental
cd "$BR/libs/container"
git show HEAD:include/$HDR > /tmp/shadow/$HDR || exit 1
python3 "$H/mkvariant.py" "$BR/$HDR" /tmp/shadow2/$HDR || exit 1

cd "$E"
: > /tmp/rows3
for cc in g++-16 clang++-22; do
   for variant in HEAD REFAC SPLIT; do
      case $variant in
         HEAD)  INC="-I/tmp/shadow -I$BR" ;;
         SPLIT) INC="-I/tmp/shadow2 -I$BR" ;;
         *)     INC="-I$BR" ;;
      esac
      for a in 16 32 64 128; do
         $cc -std=c++20 -O3 -DNDEBUG -DBENCH_ON \
             -DBOOST_CONTAINER_BENCH_SEGMENTED_GROUP=20 \
             -falign-functions=$a -falign-loops=$a \
             $INC bench_segmented_algos.cpp -o /tmp/b3.elf 2>/tmp/b3.$cc.$variant.log \
           || { echo "$cc/$variant BUILDFAIL"; grep -m1 'error:' /tmp/b3.$cc.$variant.log; break; }
         taskset -c 3 /tmp/b3.elf 2>/dev/null \
            | awk -v c="$cc" -v v="$variant" '/^search\(/ {print c, v, $1, $5, $7}' >> /tmp/rows3
      done
   done
done

awk '
  { seg[$1 " " $3 " " $2] = seg[$1 " " $3 " " $2] " " $4
    nsg[$1 " " $3 " " $2] = nsg[$1 " " $3 " " $2] " " $5
    seen[$1 " " $3] = 1 }
  function med(s) { m = split(s, v, " ")
     for (x=1; x<m; x++) for (y=x+1; y<=m; y++) if (v[y]+0 < v[x]+0) {t=v[x];v[x]=v[y];v[y]=t}
     return (m % 2) ? v[(m+1)/2] : (v[m/2]+v[m/2+1])/2 }
  END {
     printf "%-12s %-13s | %-21s | %s\n", "compiler", "row", "seg ns (H/R/S)", "nsg ns (H/R/S)  R-vs-H  S-vs-H"
     for (k in seen) {
        split(k, p, " ")
        sh=med(seg[k " HEAD"]); sr=med(seg[k " REFAC"]); ss=med(seg[k " SPLIT"])
        nh=med(nsg[k " HEAD"]); nr=med(nsg[k " REFAC"]); ns=med(nsg[k " SPLIT"])
        printf "%-12s %-13s | %5.3f %5.3f %5.3f     | %5.3f %5.3f %5.3f  %+6.1f%% %+6.1f%%\n",
               p[1], p[2], sh, sr, ss, nh, nr, ns, (nr/nh-1)*100, (ns/nh-1)*100
     }
  }' /tmp/rows3 | sort
