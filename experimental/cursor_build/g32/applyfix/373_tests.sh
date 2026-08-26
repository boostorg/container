#!/bin/bash
set -u
BR=/mnt/d/Data/LocalGit/boost
EX=$BR/libs/container/experimental
S=$EX/cursor_build/g32/applyfix
O=/tmp/g32tests
mkdir -p $O

TESTS="copy_if remove_copy remove_copy_if remove remove_if partition_copy copy transform copy_n reverse_copy swap_ranges merge set_union set_difference set_intersection set_symmetric_difference stable_partition partition"

fails=0
total=0
echo "=============== matrix: -O2 -DNDEBUG -Wall -Wextra, zero warnings required ==============="
printf "%-26s" "test"
for CXX in g++-16 clang++-22; do for STD in 03 11 17 20; do printf " %-9s" "$CXX/$STD"; done; done
echo
for t in $TESTS; do
   printf "%-26s" "$t"
   for CXX in g++-16 clang++-22; do
      for STD in 03 11 17 20; do
         total=$((total+1))
         log=$O/$t.$CXX.$STD.log
         if $CXX -std=c++$STD -O2 -DNDEBUG -Wall -Wextra -I$BR -I$EX \
               $EX/segmented_${t}_test.cpp -o $O/t.elf > $log 2>&1; then
            if [ -s $log ]; then
               printf " %-9s" "WARN"; fails=$((fails+1))
            elif $O/t.elf >> $log 2>&1; then
               printf " %-9s" "pass"
            else
               printf " %-9s" "RUNFAIL"; fails=$((fails+1))
            fi
         else
            printf " %-9s" "BUILDERR"; fails=$((fails+1))
         fi
      done
   done
   echo
done
echo "matrix: $total configurations, $fails not clean"

echo
echo "=============== first diagnostic of each non-clean configuration ==============="
for f in $O/*.log; do
   if [ -s "$f" ]; then echo "--- $f"; head -6 "$f" | sed 's/^/    /'; fi
done

echo
echo "=============== sanitisers: clang++-22 -std=c++20 -fsanitize=address,undefined -O1 ==============="
SAN="copy_if remove_copy remove_copy_if remove remove_if partition_copy copy"
for t in $SAN; do
   if clang++-22 -std=c++20 -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer \
         -Wall -Wextra -I$BR -I$EX $EX/segmented_${t}_test.cpp -o $O/s.elf > $O/san.$t.log 2>&1; then
      if $O/s.elf >> $O/san.$t.log 2>&1; then printf "%-22s clean\n" "$t"
      else printf "%-22s SANITIZER/RUN FAIL\n" "$t"; tail -20 $O/san.$t.log | sed 's/^/    /'; fi
   else
      printf "%-22s BUILDERR\n" "$t"; head -10 $O/san.$t.log | sed 's/^/    /'
   fi
done
echo "--- probe under sanitisers ---"
if clang++-22 -std=c++20 -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer \
      -Wall -Wextra -I$BR -I$EX $S/370_probe.cpp -o $O/sp.elf > $O/san.probe.log 2>&1; then
   if $O/sp.elf > $O/san.probe.out 2>>$O/san.probe.log; then
      echo "probe clean, mismatches=$(grep -c MISMATCH $O/san.probe.out)"
   else
      echo "probe FAILED"; tail -20 $O/san.probe.log | sed 's/^/    /'
   fi
else
   echo "probe BUILDERR"; head -10 $O/san.probe.log | sed 's/^/    /'
fi
echo "--- probe under g++-16 sanitisers ---"
if g++-16 -std=c++20 -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer \
      -Wall -Wextra -I$BR -I$EX $S/370_probe.cpp -o $O/gp.elf > $O/gsan.probe.log 2>&1; then
   if $O/gp.elf > $O/gsan.probe.out 2>>$O/gsan.probe.log; then
      echo "probe clean, mismatches=$(grep -c MISMATCH $O/gsan.probe.out)"
   else
      echo "probe FAILED"; tail -20 $O/gsan.probe.log | sed 's/^/    /'
   fi
else
   echo "probe BUILDERR"; head -10 $O/gsan.probe.log | sed 's/^/    /'
fi

echo
echo "=============== g22 depth survey (202_survey.cpp) ==============="
D=$EX/cursor_build/g22/deep
NAMES=(x copy copy_if copy_n transform remove_copy remove_copy_if reverse_copy \
       swap_ranges merge set_union set_difference set_intersection \
       set_symmetric_difference partition_copy)
printf "%-26s %-10s %-10s\n" "algorithm" "depth1" "depth2"
for i in $(seq 1 14); do
   res=()
   for d in 1 2; do
      if g++-16 -std=c++20 -O1 -DNDEBUG -DALGO=$i -DDEPTH=$d -I$BR -I$EX \
            -c $D/202_survey.cpp -o $O/a.o 2>$O/e.$i.$d.log; then res+=("ok"); else res+=("FAIL"); fi
   done
   printf "%-26s %-10s %-10s\n" "${NAMES[$i]}" "${res[0]}" "${res[1]}"
done
