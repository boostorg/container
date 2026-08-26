#!/bin/bash
# ASan + UBSan on the given tests, g++-16 and clang++-22, -O1 -g.
ROOT=/mnt/d/Data/LocalGit/boost
TD=$ROOT/libs/container/experimental
OUT=/tmp/pfsan
rm -rf $OUT
mkdir -p $OUT
FLAGS="-O1 -g -fno-omit-frame-pointer -fsanitize=address,undefined -fno-sanitize-recover=all -DNDEBUG -Wall -Wextra -I$ROOT -I$TD"
rc_all=0
for t in "$@"; do
  for cc in g++-16 clang++-22; do
    tag="$t.$cc"
    if ! $cc -std=c++17 $FLAGS "$TD/$t.cpp" -o "$OUT/$tag.elf" > "$OUT/$tag.cc.log" 2>&1; then
      echo "BUILD-FAIL $tag"; head -20 "$OUT/$tag.cc.log"; rc_all=1; continue
    fi
    if "$OUT/$tag.elf" > "$OUT/$tag.run.log" 2>&1; then
      echo "SAN-OK     $tag"
    else
      echo "SAN-FAIL   $tag"; head -40 "$OUT/$tag.run.log"; rc_all=1
    fi
  done
done
exit $rc_all
