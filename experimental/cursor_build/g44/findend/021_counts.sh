#!/bin/bash
set -u
D=/mnt/d/Data/LocalGit/boost/libs/container/experimental
cd "$D" || exit 1
OUT="$D/cursor_build/g44/findend/out"
mkdir -p "$OUT"
g++-16 -std=c++17 -Wall -Wextra -O1 -I../../.. -I"$D" \
   cursor_build/g44/findend/020_counts.cpp -o "$OUT/counts.elf" 2>&1 | head -40
[ -x "$OUT/counts.elf" ] && "$OUT/counts.elf"
