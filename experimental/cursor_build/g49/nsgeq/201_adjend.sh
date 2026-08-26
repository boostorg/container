#!/bin/bash
set -u
BR=/mnt/d/Data/LocalGit/boost
CB=$BR/libs/container/bench
H=$BR/libs/container/experimental/cursor_build/g49/nsgeq
O=/tmp/g49_adj_$$
mkdir -p "$O"

for CXX in g++-16 clang++-22; do
   echo "########## $CXX ##########"
   $CXX -std=c++20 -O3 -DNDEBUG -I"$BR" -I"$CB" "$H/200_adjend.cpp" -o "$O/a.elf" \
      || { echo BUILDFAIL; continue; }
   taskset -c 3 "$O/a.elf"
   taskset -c 3 "$O/a.elf"
done
