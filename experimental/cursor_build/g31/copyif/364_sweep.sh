#!/bin/bash
set -u
BR=/mnt/d/Data/LocalGit/boost
WD=$BR/libs/container/experimental/cursor_build/g31/copyif
cd "$WD" || exit 1

FLAGS="-std=c++20 -O3 -I$BR -DNDEBUG -DBENCH_ON -falign-functions=64 -falign-loops=64"

for CXX in g++-16 clang++-22; do
  echo "########## $CXX ##########"
  $CXX $FLAGS 364_sweep.cpp -o 364_${CXX}.elf 2>&1 | head -30
  if [ -x 364_${CXX}.elf ]; then
    nice -n -5 taskset -c 3 ./364_${CXX}.elf
  else
    echo BUILD FAILED
  fi
  echo
done
