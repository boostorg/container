#!/bin/bash
# SIMD / unroll fingerprint of the count and find leaf loops per variant.
set -u
cd /mnt/d/Data/LocalGit/boost/libs/container/experimental/cursor_build/g28/pattern/out
for CC in clang++-22 g++-16; do
   for P in probe_count_deq probe_find_deq probe_fill_deq; do
      for v in base T F H; do
         body=$(awk "/$P/{f=1} f&&/^\$/{exit} f" lst.$CC.$v-O2.txt)
         simd=$(echo "$body" | grep -cE 'paddd|vpaddd|pcmpeq|vpcmpeq|movdqu|vmovdq')
         insns=$(echo "$body" | grep -cE '^\s+[0-9a-f]+:')
         cbound=$(echo "$body" | grep -c '0x400')
         printf "%-12s %-18s %-5s insns=%-5s simd=%-4s const-bound=%s\n" $CC $P $v "$insns" "$simd" "$cbound"
      done
   done
done
