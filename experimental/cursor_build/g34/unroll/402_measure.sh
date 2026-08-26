#!/bin/bash
set -u
G34=/mnt/d/Data/LocalGit/boost/libs/container/experimental/cursor_build/g34/unroll
OUT=$G34/runs; mkdir -p $OUT

echo "=== smoke test both proto binaries ==="
taskset -c 3 $G34/gcc_proto.elf > $OUT/smoke_g.txt 2>$OUT/smoke_g.err; echo "gcc_proto exit=$?  lines=$(wc -l < $OUT/smoke_g.txt)  err=$(cat $OUT/smoke_g.err)"
taskset -c 3 $G34/clang_proto.elf > $OUT/smoke_c.txt 2>$OUT/smoke_c.err; echo "clang_proto exit=$? lines=$(wc -l < $OUT/smoke_c.txt) err=$(cat $OUT/smoke_c.err)"

for cfg in gcc_proto clang_proto; do
  for i in 1 2 3 4 5; do taskset -c 3 $G34/$cfg.elf > $OUT/$cfg.$i.txt 2>&1; done
  echo "$cfg runs done"
done

med() { sort -g | awk '{a[NR]=$1} END{print a[int((NR+1)/2)]}'; }
for cfg in gcc_proto clang_proto; do
  echo
  echo "############ MEDIAN OF 5: $cfg ############"
  printf '%-28s%12s%12s%12s%11s%11s%11s\n' '< algo >' '<nsg/seg>' '<std/seg>' '<std/nsg>' '<seg ns>' '<std ns>' '<nsg ns>'
  grep -hE '^(copy|remove|swap|transform)' $OUT/$cfg.1.txt | cut -c1-28 | while IFS= read -r lab; do
    key=$(echo "$lab" | sed 's/ *$//')
    s=$(for i in 1 2 3 4 5; do grep -F "$key " $OUT/$cfg.$i.txt|head -1|awk '{print $(NF-2)}'; done|med)
    t=$(for i in 1 2 3 4 5; do grep -F "$key " $OUT/$cfg.$i.txt|head -1|awk '{print $(NF-1)}'; done|med)
    n=$(for i in 1 2 3 4 5; do grep -F "$key " $OUT/$cfg.$i.txt|head -1|awk '{print $NF}'; done|med)
    awk -v l="$key" -v s="$s" -v t="$t" -v n="$n" 'BEGIN{printf "%-28s%12.2f%12.2f%12.2f%11.3f%11.3f%11.3f\n",l,n/s,t/s,t/n,s,t,n}'
  done
done
echo DONE
