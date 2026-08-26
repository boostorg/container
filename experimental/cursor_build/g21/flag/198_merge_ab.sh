#!/bin/bash
set -u
BR=/mnt/d/Data/LocalGit/boost
S=$BR/libs/container/experimental/cursor_build/g21/flag
EX=$BR/libs/container/experimental
O=/tmp/g21ab
mkdir -p $O

echo "############################ codegen ############################"
for CXX in g++-16 clang++-22; do
   $CXX -std=c++20 -O2 -DNDEBUG -I$BR -I$EX -c $S/195_deep_cg.cpp -o $O/d.$CXX.o 2>$O/d.$CXX.log \
      || { echo "$CXX BUILD FAIL"; grep -m6 'error:' $O/d.$CXX.log; continue; }
   objdump -d --no-show-raw-insn $O/d.$CXX.o > $O/d.$CXX.asm
   echo "-- $CXX --"
   for want in mrg1 mrg2 merge_dst_bounded merge_until_exhausts; do
      grep -nE "^[0-9a-f]+ <.*$want.*>:\$" $O/d.$CXX.asm | while IFS=: read -r ln rest; do
         mang=${rest#*<}; mang=${mang%>:}
         short=$(echo "$mang" | c++filt 2>/dev/null | sed -E 's/.*detail_algo:://; s/[(<].*//')
         [ -z "$short" ] && short="$mang"
         body=$(awk -v s="$ln" 'NR>s { if ($0 ~ /^$/) exit; print }' $O/d.$CXX.asm)
         tot=$(printf '%s\n' "$body" | grep -cE '^\s+[0-9a-f]+:')
         c=$(printf '%s\n' "$body" | grep -cE '^\s+[0-9a-f]+:\s+(cmp|test)')
         printf "   %-26s insns=%-5s cmp/test=%s\n" "$short" "$tot" "$c"
      done
   done | sort -u
done

echo
echo "############################ tests ############################"
for CXX in g++-16 clang++-22; do
   for std in c++03 c++11 c++17 c++20; do
      LOG=$O/t.$CXX.$std.log
      $CXX -std=$std -O2 -DNDEBUG -I$BR -Wall -Wextra $EX/segmented_merge_test.cpp \
           -o $O/t.$CXX.$std.elf 2>$LOG
      brc=$?
      w=$(grep -c 'warning:' $LOG)
      if [ $brc -ne 0 ]; then
         printf "   %-12s %-7s BUILD FAIL\n" "$CXX" "$std"; grep -m3 'error:' $LOG; continue
      fi
      $O/t.$CXX.$std.elf >/dev/null 2>&1
      rrc=$?
      printf "   %-12s %-7s warnings=%-3s run_exit=%s\n" "$CXX" "$std" "$w" "$rrc"
   done
done

echo
echo "###################### sanitised (c++20) ######################"
for CXX in g++-16 clang++-22; do
   $CXX -std=c++20 -O1 -g -fsanitize=address,undefined -I$BR \
        $EX/segmented_merge_test.cpp -o $O/s.$CXX.elf 2>/dev/null \
      && { printf "   %-12s " "$CXX"; $O/s.$CXX.elf >/dev/null 2>&1 && echo "clean" || echo "SANITISER/TEST FAILURE"; }
done
