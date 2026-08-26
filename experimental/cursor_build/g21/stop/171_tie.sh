#!/bin/bash
set -u
BR=/mnt/d/Data/LocalGit/boost
P=$BR/libs/container/experimental/cursor_build/g21/stop/170_tie_probe.cpp
OUT=/tmp/g21tie
mkdir -p $OUT

echo "############ plain -O2 ############"
for CXX in g++-16 clang++-22; do
   $CXX -std=c++20 -O2 -DNDEBUG -I$BR -Wall -Wextra "$P" -o $OUT/t.$CXX.elf 2>$OUT/$CXX.log \
      || { echo "$CXX BUILD FAIL"; head -20 $OUT/$CXX.log; continue; }
   echo "-- $CXX (warnings=$(grep -c 'warning:' $OUT/$CXX.log)) --"
   $OUT/t.$CXX.elf
   echo "exit=$?"
done

echo
echo "############ ASan + UBSan ############"
for CXX in g++-16 clang++-22; do
   $CXX -std=c++20 -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer \
        -I$BR "$P" -o $OUT/s.$CXX.elf 2>$OUT/s$CXX.log \
      || { echo "$CXX SAN BUILD FAIL"; head -20 $OUT/s$CXX.log; continue; }
   echo "-- $CXX --"
   $OUT/s.$CXX.elf 2>&1 | head -30
   echo "exit=$?"
done
