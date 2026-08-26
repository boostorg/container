#!/bin/bash
cd /mnt/d/Data/LocalGit/boost/libs/container/experimental/cursor_build/g28/pattern/out/bench
for v in base H F T; do
   echo "== $v"
   grep -h '^copy(2S)' bench.clang++-22.$v.g25.run*.txt | awk '{print $5}' | paste -sd' '
done
