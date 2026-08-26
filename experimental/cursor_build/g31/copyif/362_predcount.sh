#!/bin/bash
set -u
BR=/mnt/d/Data/LocalGit/boost
WD=$BR/libs/container/experimental/cursor_build/g31/copyif
cd "$WD" || exit 1

for CXX in g++-16 clang++-22; do
  echo "########## $CXX ##########"
  $CXX -std=c++20 -O2 -I"$BR" -DNDEBUG 362_predcount.cpp -o 362_${CXX}.elf 2>&1 | head -40
  if [ -x 362_${CXX}.elf ]; then ./362_${CXX}.elf; else echo "BUILD FAILED"; fi
  echo
done
