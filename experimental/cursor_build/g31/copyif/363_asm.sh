#!/bin/bash
# 363: locate the copy_if destination walker / leaf symbols in the benchmark
# binaries and dump them.
set -u
BR=/mnt/d/Data/LocalGit/boost
EX=$BR/libs/container/experimental
WD=$EX/cursor_build/g31/copyif
cd "$WD" || exit 1

for CXX in g++-16 clang++-22; do
  echo "########################## $CXX ##########################"
  nm -C "a_${CXX}.elf" 2>/dev/null | grep -i 'copy_if' | sed 's/^/SYM  /' | head -40
  echo "--- mangled ---"
  nm "a_${CXX}.elf" 2>/dev/null | grep -i 'copy_if' | head -40
  echo
done
