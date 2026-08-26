#!/bin/bash
# Quick single-config compile+run of the new find_end test.
set -u
cd /mnt/d/Data/LocalGit/boost/libs/container/experimental || exit 1
OUT=/mnt/d/Data/LocalGit/boost/libs/container/experimental/cursor_build/g44/findend/out
mkdir -p "$OUT"

g++-16 -std=c++20 -Wall -Wextra -O1 -I../../.. \
   segmented_find_end_test.cpp -o "$OUT/quick.elf" 2>&1 | head -80
if [ -x "$OUT/quick.elf" ]; then
   "$OUT/quick.elf" && echo "RUN OK" || echo "RUN FAIL"
else
   echo "BUILD FAIL"
fi
