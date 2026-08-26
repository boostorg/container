#!/bin/bash
set -u
BR=/mnt/d/Data/LocalGit/boost
CB=$BR/libs/container/bench
H=$BR/libs/container/experimental/cursor_build/g42/searchperf
for cc in g++-16 clang++-22; do
   echo "================ $cc ================"
   if $cc -std=c++17 -O2 -DNDEBUG -I"$BR" -I"$CB" "$H/probe2.cpp" \
        -o "/tmp/p2.$cc.elf" 2>"/tmp/e2.$cc.log"; then
      [ -s "/tmp/e2.$cc.log" ] && { echo "--- diags ---"; sed -n '1,12p' "/tmp/e2.$cc.log"; }
      taskset -c 3 "/tmp/p2.$cc.elf"
   else
      echo BUILDFAIL
      sed -n '1,30p' "/tmp/e2.$cc.log"
   fi
done
