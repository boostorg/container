#!/bin/bash
# Extract one inner-loop copy from the 2S (vector, deque) std and nsg
# measure_batch bodies.
set -u
cd /mnt/d/Data/LocalGit/boost/libs/container/experimental/cursor_build/g49/nsgeq

for pat in "measure_batch<bench_ops::std_equal<boost::container::vector" \
           "measure_batch<bench_ops::seg_equal<boost::container::vector"; do
   echo "################ $pat ################"
   # Symbol bodies matching; for seg_equal print both (seg then nsg).
   awk -v s="$pat" '
      /^0[0-9a-f]+ </ { inb = index($0, s) > 0; if(inb) { print "=== " substr($0, 1, 60) " ... ==="; n = 0 } }
      inb && /bench_utils.hpp:168/ { grab = 14; n++ }
      inb && grab > 0 && n == 2 { print; grab-- }
   ' bench_clang.asm | grep -vE "^/mnt|^unsigned|^bench_ops|^boost::" | head -44
done
