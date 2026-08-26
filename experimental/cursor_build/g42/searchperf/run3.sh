#!/bin/bash
set -u
BR=/mnt/d/Data/LocalGit/boost
CB=$BR/libs/container/bench
H=$BR/libs/container/experimental/cursor_build/g42/searchperf
for cc in g++-16 clang++-22; do
   echo "================ $cc ================"
   if $cc -std=c++17 -O2 -DNDEBUG -I"$BR" -I"$CB" "$H/probe3.cpp" \
        -o "/tmp/p3.$cc.elf" 2>"/tmp/e3.$cc.log"; then
      [ -s "/tmp/e3.$cc.log" ] && { echo "--- diags ---"; sed -n '1,12p' "/tmp/e3.$cc.log"; }
      taskset -c 3 "/tmp/p3.$cc.elf"
   else
      echo BUILDFAIL
      sed -n '1,30p' "/tmp/e3.$cc.log"
   fi
done
