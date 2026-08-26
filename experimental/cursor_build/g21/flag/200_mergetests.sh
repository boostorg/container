#!/bin/bash
set -u
BR=/mnt/d/Data/LocalGit/boost
EX=$BR/libs/container/experimental
cd $EX || exit 1
for t in *merge*test.cpp; do
   for CXX in g++-16 clang++-22; do
      if $CXX -std=c++20 -O2 -DNDEBUG -I$BR -Wall -Wextra "$t" -o /tmp/tt.elf 2>/tmp/tt.log; then
         if /tmp/tt.elf >/dev/null 2>&1; then
            printf "  %-12s %-34s PASS  warnings=%s\n" "$CXX" "$t" "$(grep -c 'warning:' /tmp/tt.log)"
         else
            printf "  %-12s %-34s RUN FAIL\n" "$CXX" "$t"
         fi
      else
         printf "  %-12s %-34s BUILD FAIL\n" "$CXX" "$t"
         grep -m3 'error:' /tmp/tt.log
      fi
   done
done
