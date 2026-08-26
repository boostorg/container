#!/bin/bash
set -u
cd /mnt/d/Data/LocalGit/boost/libs/container/experimental/cursor_build/g49/nsgeq

echo "################ nsg mismatch_2r D+D: counted leaf loop ################"
sed -n '5656,7471p' bench_gcc.asm \
   | awk '/segmented_common_algo.hpp:452/ {n++} n==2 {print} n==2 && /jne|jmp/ {c++} c>7 {exit}' \
   | grep -E '^ ' | head -40

echo "################ std mismatch_2r D+D: std::mismatch loop ################"
sed -n '8742,10809p' bench_gcc.asm \
   | awk '/stl_algobase.h:20/ {n++} n>=3 {print} n>=3 && /jne|jmp|je / {c++} c>9 {exit}' \
   | grep -E '^ ' | head -40
