#!/bin/bash
set -u
cd /mnt/d/Data/LocalGit/boost/libs/container/experimental/cursor_build/g49/nsgeq
echo "===== full names of out-of-line leaves/walkers (GCC) ====="
for a in 25a0 2720 28a0 2990 2b20 2cc0 2e40 2f60 31e0; do
   grep -E "^0+$a <" bench_gcc.asm | sed -E "s/boost::container:://g; s/MyInt\*, true, 0u, 128u, unsigned long/DQIT/g; s/MyInt\*, true/VECIT/g; s/, void//g; s/deque_opt<0ul, 128ul, false>/OPT/g; s/detail_algo:://g" | cut -c1-400
   echo
done
echo "===== who calls the out-of-line leaves ====="
for a in 25a0 2720; do
   echo "--- callers of $a ---"
   grep -n "call   $a <" bench_gcc.asm | wc -l
   # find enclosing symbol of each call
   awk -v tgt="call   $a <" '
      /^0[0-9a-f]+ </ { sym = $0 }
      index($0, tgt) { print substr(sym, 1, 130) }
   ' bench_gcc.asm | sort | uniq -c
done
