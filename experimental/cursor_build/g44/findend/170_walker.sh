#!/bin/bash
# Disassembles the outlined one-level verify walker, alignment padding removed.
set -u
D=/mnt/d/Data/LocalGit/boost/libs/container/experimental/cursor_build/g44/findend/out/codegen
for v in "$@"; do
  echo "=================== $v"
  awk '
    /^[0-9a-f]+ <.*>:$/ { p = (index($0,"find_end_verify") && index($0,"seg_vector_iterator<int") && !index($0,"seg2_vector_iterator")) }
    p && !/nop|xchg   %ax,%ax/ { if (/^[0-9a-f]+ </) print "--- walker"; else print }
  ' "$D/$v.dis"
done
