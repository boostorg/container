#!/bin/bash
set -u

dump_body() { # asmfile startline title
   local f=$1 start=$2 title=$3
   echo "################ $title ################"
   # find next symbol after start to bound
   end=$(awk -v s="$start" 'NR>s && /^0[0-9a-f]+ </{print NR; exit}' "$f")
   [ -n "$end" ] || end=$((start+800))
   sed -n "${start},$((end-1))p" "$f" \
      | sed -E 's/<unsigned long measure_batch[^>]*\+/<mb+/; s/^\/mnt.*\/([A-Za-z0-9_.]+):([0-9]+)$/;; \1:\2/' \
      > /tmp/body.txt
   echo "instructions: $(grep -cE '^    [0-9a-f]+:' /tmp/body.txt)"
   echo "calls: $(grep -cE 'call ' /tmp/body.txt)"
   grep -E 'call ' /tmp/body.txt | sed -E 's/.*call\s+//; s/<boost::container::detail_algo::/<da::/; s/\(.*?\)//' | head -6
   echo "line census:"
   grep -oE '[A-Za-z0-9_.]+\.(hpp|h):[0-9]+' /tmp/body.txt | sed 's|.*/||' | sort | uniq -c | sort -rn | head -12
   echo "SIMD/vector ops:"
   grep -cE 'xmm|ymm|zmm|movdqu|movaps|vmov' /tmp/body.txt || true
   echo "hot loop candidates (test/jcc dense):"
   # show region around is_odd / copy_if pred
   awk '/segmented_copy_if\.hpp:|stl_algo\.h:|is_odd|bench_utils\.hpp:16[0-9]/ {hit=1}
        hit && /^    [0-9a-f]+:/ {print; n++; if(n>40) exit}' /tmp/body.txt
   echo
}

echo "===== GCC ====="
dump_body /tmp/g25_g++-16.asm 13075 "GCC SEG V+D hit"
dump_body /tmp/g25_g++-16.asm 28859 "GCC STD V+D hit"

echo "===== CLANG ====="
dump_body /tmp/g25_clang++-22.asm 31009 "CLANG SEG V+D hit"
dump_body /tmp/g25_clang++-22.asm 29561 "CLANG STD V+D hit"

# Out-of-line dst_dispatch if any
echo "===== out-of-line copy_if helpers ====="
for CXX in g++-16 clang++-22; do
   echo "--- $CXX ---"
   grep -E "^0[0-9a-f]+ <" /tmp/g25_${CXX}.asm | grep "detail_algo::segmented_copy_if" \
      | sed -E 's/^0*([0-9a-f]+) <.*(segmented_copy_if_[a-z_]+).*/\1 \2/' 
done
