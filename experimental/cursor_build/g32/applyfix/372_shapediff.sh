#!/bin/bash
set -u
S=/mnt/d/Data/LocalGit/boost/libs/container/experimental/cursor_build/g32/applyfix
cd $S
for c in "g++-16" "clang++-22"; do
   echo "=== $c : SHAPE pre vs post ==="
   if diff -u "shape_pre_$c.txt" "shape_post_$c.txt" > /tmp/d.txt; then
      echo "IDENTICAL ($(wc -l < shape_post_$c.txt) lines)"
   else
      echo "DIFFERS ($(grep -c '^[+-]' /tmp/d.txt) changed lines)"
      head -80 /tmp/d.txt
   fi
done
