#!/bin/bash
set -u
S=/mnt/d/Data/LocalGit/boost/libs/container/experimental/cursor_build/g32/applyfix
agg() {
   awk '/^COUNT/{
      algo=$2; dst="";
      for(i=1;i<=NF;i++) if($i ~ /^dst=/) dst=substr($i,5);
      for(i=1;i<=NF;i++){ if($i ~ /^applied=/) a=substr($i,9); if($i ~ /^mandated=/) m=substr($i,10); }
      key=algo" "dst; tot[key]+=a; man[key]+=m; if(!(key in seen)){seen[key]=1; ord[++k]=key}
   } END{ for(i=1;i<=k;i++) printf "%s|%d|%d\n", ord[i], tot[ord[i]], man[ord[i]] }' "$1"
}
for CXX in g++-16 clang++-22; do
   echo "=================== $CXX ==================="
   agg $S/count_pre_$CXX.txt  > /tmp/pre.$CXX
   agg $S/count_now_$CXX.txt  > /tmp/now.$CXX
   printf "%-40s %10s %10s %10s\n" "algorithm / destination" "mandated" "before" "after"
   join -t'|' /tmp/pre.$CXX /tmp/now.$CXX | awk -F'|' '{
      printf "%-40s %10d %10d %10d%s\n", $1, $3, $2, $4, ($4!=$3 ? "   STILL OFF" : ($2!=$3 ? "   fixed" : ""))
   }'
done
