#!/bin/bash
set -u
G34=/mnt/d/Data/LocalGit/boost/libs/container/experimental/cursor_build/g34/unroll
for cfg in clang_off gcc_off; do
  echo "################ $cfg : measure_batch instantiations (copy_if / copy_n) ################"
  nm -C --size-sort -S $G34/$cfg.elf 2>/dev/null | grep -i "measure_batch" | grep -Ei "copy_if|copy_n" | \
    sed 's/boost::container::bench_ops:://g; s/boost::move_detail:://g; s/boost::container:://g' | \
    while IFS= read -r l; do echo "$l"; done
  echo
done
echo "################ raw mangled (clang_off) ################"
nm --size-sort -S $G34/clang_off.elf 2>/dev/null | grep "measure_batch" | grep -E "seg_copy_if|std_copy_if" 
echo DONE
