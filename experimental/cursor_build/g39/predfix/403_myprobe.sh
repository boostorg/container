#!/bin/bash
# Build+run the g39 probes with g++-16 -std=c++20 -O2 -DNDEBUG.
ROOT=/mnt/d/Data/LocalGit/boost
TD=$ROOT/libs/container/experimental
PD=$TD/cursor_build/g39/predfix
OUT=/tmp/pf39
mkdir -p $OUT
FLAGS="-O2 -DNDEBUG -Wall -Wextra -I$ROOT -I$TD"
for p in "$@"; do
  echo "########## $p"
  rm -f "$OUT/$p.elf"
  g++-16 -std=c++20 $FLAGS "$PD/$p.cpp" -o "$OUT/$p.elf" 2>&1 | head -40
  [ -x "$OUT/$p.elf" ] && "$OUT/$p.elf"
done
