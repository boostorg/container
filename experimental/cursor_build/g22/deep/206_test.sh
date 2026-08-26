#!/bin/bash
set -u
BR=/mnt/d/Data/LocalGit/boost
EX=$BR/libs/container/experimental
O=/tmp/g22t
mkdir -p $O
cd $EX || exit 1
for t in "$@"; do
   for CXX in g++-16 clang++-22; do
      if $CXX -std=c++20 -O2 -DNDEBUG -I$BR -Wall -Wextra "$t.cpp" -o $O/$t.$CXX.elf 2>$O/$t.$CXX.log; then
         out=$($O/$t.$CXX.elf 2>&1); rc=$?
         if [ $rc -eq 0 ]; then
            printf "  %-12s %-46s PASS  warnings=%s\n" "$CXX" "$t" "$(grep -c 'warning:' $O/$t.$CXX.log)"
         else
            printf "  %-12s %-46s RUN FAIL (exit %s)\n" "$CXX" "$t" "$rc"
            printf '%s\n' "$out" | grep -iE 'error|fail' | head -6 | sed 's/^/      /'
         fi
      else
         printf "  %-12s %-46s BUILD FAIL\n" "$CXX" "$t"
         grep -m4 'error:' $O/$t.$CXX.log | sed 's/^/      /'
      fi
   done
done
