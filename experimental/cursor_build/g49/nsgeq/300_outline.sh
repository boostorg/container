#!/bin/bash
# Out-of-line detail_algo/segmented symbols in the group-20 binaries, with
# sizes: shows what each compiler declined to inline for equal/mismatch/search.
set -u
cd /mnt/d/Data/LocalGit/boost/libs/container/experimental/cursor_build/g49/nsgeq

for f in bench_gcc.asm bench_clang.asm; do
   echo "########## $f ##########"
   grep -E "^0[0-9a-f]+ <" "$f" | grep -vE "measure_batch|@plt|std::|__gnu|compare_batch|bench_equal|bench_mismatch|bench_search|print_|calc_|main|cpu_timer|_GLOBAL|deregister|register_tm|frame_dummy|_init|_fini|_start|__libc|__do_global" \
      | grep -E "equal|mismatch|search" \
      | sed -E 's/^0*([0-9a-f]+) <([^(]{0,150}).*/\1 \2/' | head -30
done
