#!/bin/bash
set -u
BR=/mnt/d/Data/LocalGit/boost
P=$BR/libs/container/experimental/cursor_build/g21/stop/156_flagb_probe.cpp
OUT=/tmp/g21c
mkdir -p $OUT
BIN=$OUT/clang.elf
clang++-22 -std=c++20 -O3 -DNDEBUG -I$BR "$P" -o "$BIN" 2>/dev/null || exit 1
objdump -d --no-show-raw-insn -C "$BIN" > $OUT/clang.asm

echo "##### clang a_base_int (full) #####"
awk '/^[0-9a-f]+ <a_base_int\(/ {inf=1} inf {print} inf && /ret/ {c++} inf && /^$/ {exit}' $OUT/clang.asm | sed 's/<[^>]*>//g'

echo
echo "##### symbols containing walker_a #####"
nm --print-size -C "$BIN" | grep -i "walker_a" | sed 's/<[^>]*>//g' | head -20

echo
echo "##### all outlined/local text symbols over 100 bytes #####"
nm --print-size -C "$BIN" | awk '$3=="t" || $3=="T" {n=strtonum("0x" $2); if(n>100) print n, $4}' | sort -rn | head -25
