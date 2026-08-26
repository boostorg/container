#!/bin/bash
set -u
cd /mnt/d/Data/LocalGit/boost/libs/container/experimental/cursor_build/g49/nsgeq
for a in 31e0 2f60; do
   echo "################ 0x$a ################"
   awk -v s="^0+$a <" '
      $0 ~ s {p=1}
      p && /^$/ {exit}
      p {print}
   ' bench_gcc.asm | sed -E 's/<[^>]*\+(0x[0-9a-f]+)>/<+\1>/; s/^\/mnt.*\/([a-z_0-9]+\.(hpp|h)):([0-9]+)$/;; \1:\3/' \
     | grep -vE '^(boost::|segtrio|segduo|unsigned|bench_ops|std::|bool )' 
done
