#!/bin/bash
# Disassemble the clang specialisation behind the copy_if(1S miss) nsg row and
# count taken-in-the-miss-case branches, base vs unrolled.
set -u
W=/mnt/d/Data/LocalGit/boost/libs/container/experimental/cursor_build/g37/unroll
BIN=$W/bin
D=$W/dis
mkdir -p $D
# nsg = Wrap true, source deque<MyInt>, dest vector<MyInt>, pred is_negative
PAT='seg_copy_if<boost::container::deque<MyInt,.*vector<MyInt.*is_negative<MyInt>, true'
for v in base p2_final; do
   b=$BIN/b_clang++-22_${v}_g25
   sym=$(nm -C -S --defined-only $b | grep -E "measure_batch<bench_ops::$PAT" | sort -k2 | tail -1)
   echo "=== clang $v : $sym" | cut -c1-160
   addr=$(echo "$sym" | awk '{print $1}')
   size=$(echo "$sym" | awk '{print $2}')
   objdump -d --start-address=0x$addr --stop-address=$((0x$addr + 0x$size)) $b \
      > $D/clang_${v}_copyif_nsg.txt
   echo "  size=0x$size  instructions=$(grep -cE '^\s+[0-9a-f]+:' $D/clang_${v}_copyif_nsg.txt)"
   echo "  test+js/jns pairs : js=$(grep -cE '\bjs\b' $D/clang_${v}_copyif_nsg.txt) jns=$(grep -cE '\bjns\b' $D/clang_${v}_copyif_nsg.txt)"
done
echo
echo "=== base inner loop (first 24 insns after the hottest backward jump target) ==="
grep -A2 -B2 -E '\b(js|jns)\b' $D/clang_base_copyif_nsg.txt | head -40
echo
echo "=== unrolled inner loop ==="
grep -A2 -B2 -E '\b(js|jns)\b' $D/clang_p2_final_copyif_nsg.txt | head -60
