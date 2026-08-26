#!/bin/bash
set -u
BR=/mnt/d/Data/LocalGit/boost
EX=$BR/libs/container/experimental
S=$EX/cursor_build/g32/applyfix
TAG=${1:-pre}
O=/tmp/g32probe
mkdir -p $O

for CXX in g++-16 clang++-22; do
   echo "=== $CXX -std=c++20 -O2 -DNDEBUG -Wall -Wextra ==="
   if $CXX -std=c++20 -O2 -DNDEBUG -Wall -Wextra -I$BR -I$EX \
         $S/370_probe.cpp -o $O/p.$CXX 2> $O/build.$CXX.log; then
      echo "build: ok"
      if [ -s $O/build.$CXX.log ]; then
         echo "build: WARNINGS"; sed 's/^/   /' $O/build.$CXX.log
      fi
      $O/p.$CXX > $S/out_${TAG}_$CXX.txt
      echo "run rc=$?  mismatches=$(grep -c MISMATCH $S/out_${TAG}_$CXX.txt)"
      grep '^COUNT' $S/out_${TAG}_$CXX.txt > $S/count_${TAG}_$CXX.txt
      grep '^SHAPE' $S/out_${TAG}_$CXX.txt > $S/shape_${TAG}_$CXX.txt
   else
      echo "build: FAILED"; sed 's/^/   /' $O/build.$CXX.log | head -40
   fi
done

echo
echo "=== cross-compiler shape agreement ==="
if diff -q $S/shape_${TAG}_g++-16.txt $S/shape_${TAG}_clang++-22.txt >/dev/null 2>&1; then
   echo "identical"
else
   echo "DIFFER"; diff $S/shape_${TAG}_g++-16.txt $S/shape_${TAG}_clang++-22.txt | head -20
fi

echo
echo "=== count summary (g++-16), grouped ==="
awk '/^COUNT/{
   algo=$2; dst=""; for(i=1;i<=NF;i++) if($i ~ /^dst=/) dst=substr($i,5);
   for(i=1;i<=NF;i++){ if($i ~ /^applied=/) a=substr($i,9); if($i ~ /^mandated=/) m=substr($i,10); }
   key=algo" "dst; tot[key]+=a; man[key]+=m; if(!(key in seen)){ seen[key]=1; ord[++k]=key }
}
END{ for(i=1;i<=k;i++){ key=ord[i]; printf "%-40s applied=%-8d mandated=%-8d extra=%d\n", key, tot[key], man[key], tot[key]-man[key] } }' \
   $S/count_${TAG}_g++-16.txt
