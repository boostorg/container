#!/bin/bash
# Prints the disassembly of one symbol for the requested variant builds.
set -u
D=/mnt/d/Data/LocalGit/boost/libs/container/experimental/cursor_build/g44/findend/out/codegen
SYM=$1; shift
for f in "$@"; do
  echo "=================== $f : $SYM"
  awk -v s="<$SYM>:" 'index($0,s){f=1;next} f&&/^$/{exit} f' "$D/$f.dis" | grep -v 'nop\|xchg   %ax,%ax'
done
