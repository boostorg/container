#!/bin/bash
set -u
cd /mnt/d/Data/LocalGit/boost/libs/container/experimental/cursor_build/g49/nsgeq
for f in bench_gcc.asm bench_clang.asm; do
   echo "##### $f #####"
   grep -E "^0[0-9a-f]+ <" "$f" | grep "detail_algo" \
      | sed -E 's/^0*([0-9a-f]+) <.*(detail_algo::[a-z_0-9]+)<.*/\1 \2/'
done
