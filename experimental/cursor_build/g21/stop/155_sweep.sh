#!/bin/bash
set -u
BR=/mnt/d/Data/LocalGit/boost
P=$BR/libs/container/experimental/cursor_build/g21/stop/154_blocksweep.cpp

for CXX in g++-16 clang++-22; do
   echo "==================== $CXX ===================="
   LOG=/tmp/sw_$CXX.log
   BIN=/tmp/sw_$CXX.elf
   $CXX -std=c++20 -O3 -DNDEBUG -I$BR -Wall -Wextra "$P" -o "$BIN" 2>$LOG
   if [ $? -ne 0 ]; then
      head -30 $LOG
      continue
   fi
   echo "warnings=$(grep -c 'warning:' $LOG)"
   taskset -c 4 "$BIN"
done
