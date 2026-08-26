#!/bin/bash
set -u
BR=/mnt/d/Data/LocalGit/boost
EX=$BR/libs/container/experimental

for CXX in g++-16 clang++-22; do
   for t in segmented_mismatch_test segmented_equal_test segmented_search_test; do
      LOG=/tmp/$t.$CXX.log
      $CXX -std=c++20 -O2 -DNDEBUG -I$BR -Wall -Wextra $EX/$t.cpp -o /tmp/$t.$CXX.elf 2>$LOG
      brc=$?
      w=$(grep -c 'warning:' $LOG)
      if [ $brc -ne 0 ]; then
         printf "  %-12s %-26s BUILD FAIL\n" "$CXX" "$t"
         grep -m3 'error:' $LOG
         continue
      fi
      /tmp/$t.$CXX.elf >/dev/null 2>&1
      rrc=$?
      printf "  %-12s %-26s warnings=%-3s run_exit=%s\n" "$CXX" "$t" "$w" "$rrc"
   done
done
