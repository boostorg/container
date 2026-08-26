#!/bin/bash
# Print the hot inner loop (2nd operator== site) of selected GCC measure_batch
# bodies: nsg mismatch_2r deque+deque (lines 16299+) and std (17201+).
set -u
cd /mnt/d/Data/LocalGit/boost/libs/container/experimental/cursor_build/g49/nsgeq

show() { # start_line end_line title
   echo "################ $3 ################"
   sed -n "${1},${2}p" bench_gcc.asm \
      | awk '/bench_utils.hpp:168/ {n++} n==2 {grab=1} grab {print} grab && /ret|jmp .*measure_batch.*\+0x[0-9a-f]{2,}>$/ {c++} c>4 {exit}' \
      | grep -vE '^/mnt|^/usr|^unsigned|^bench_ops|^boost::|^std::|^bool|^MyInt' | head -50
}

show 16299 17200 "nsg mismatch_2r deque+deque (0xa910)"
show 17201 18500 "std mismatch_2r deque+deque (0xb060)"
