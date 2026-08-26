#!/bin/bash
set -u
BR=/mnt/d/Data/LocalGit/boost
EX=$BR/libs/container/experimental
S=$EX/cursor_build/g32/applyfix
O=/tmp/g32final
mkdir -p $O

echo "############ 1. conformance probe against the CURRENT tree ############"
for CXX in g++-16 clang++-22; do
   if $CXX -std=c++20 -O2 -DNDEBUG -Wall -Wextra -I$BR -I$EX $S/370_probe.cpp -o $O/p.$CXX 2> $O/pb.$CXX.log; then
      [ -s $O/pb.$CXX.log ] && { echo "$CXX probe WARNINGS:"; sed 's/^/   /' $O/pb.$CXX.log; }
      $O/p.$CXX > $S/count_now_$CXX.txt
      echo "$CXX: rc=$? mismatches=$(grep -c MISMATCH $S/count_now_$CXX.txt)"
   else
      echo "$CXX probe BUILD FAILED"; head -20 $O/pb.$CXX.log
   fi
done
grep '^SHAPE' $S/count_now_g++-16.txt > $S/shape_now.txt
echo -n "SHAPE vs the pre-change baseline captured earlier: "
if diff -q $S/shape_pre_g++-16.txt $S/shape_now.txt > /dev/null; then echo "IDENTICAL"; else echo "DIFFERS"; diff $S/shape_pre_g++-16.txt $S/shape_now.txt | head -20; fi
echo
echo "--- per algorithm/destination, applied vs mandated (current tree) ---"
awk '/^COUNT/{
   algo=$2; dst="";
   for(i=1;i<=NF;i++) if($i ~ /^dst=/) dst=substr($i,5);
   for(i=1;i<=NF;i++){ if($i ~ /^applied=/) a=substr($i,9); if($i ~ /^mandated=/) m=substr($i,10); }
   key=algo" "dst; tot[key]+=a; man[key]+=m; if(!(key in seen)){ seen[key]=1; ord[++k]=key }
}
END{ for(i=1;i<=k;i++){ key=ord[i]; printf "%-40s applied=%-8d mandated=%-8d extra=%d\n", key, tot[key], man[key], tot[key]-man[key] } }' \
   $S/count_now_g++-16.txt

echo
echo "############ 2. set_*/merge comparison-count survey (untouched family) ############"
if g++-16 -std=c++20 -O2 -DNDEBUG -Wall -Wextra -I$BR -I$EX $S/376_setprobe.cpp -o $O/sp 2> $O/sp.log; then
   [ -s $O/sp.log ] && { echo "WARNINGS:"; sed 's/^/   /' $O/sp.log; }
   $O/sp | tee $S/setcounts.txt | grep -v '^$'
else
   echo "BUILD FAILED"; head -25 $O/sp.log
fi

echo
echo "############ 3. test matrix against the CURRENT tree ############"
TESTS="copy_if remove_copy remove_copy_if remove remove_if partition_copy copy transform copy_n reverse_copy swap_ranges merge set_union set_difference set_intersection set_symmetric_difference stable_partition partition"
fails=0; total=0; bad=""
printf "%-26s" "test"
for CXX in g++-16 clang++-22; do for STD in 03 11 17 20; do printf " %-8s" "$CXX/$STD"; done; done
echo
for t in $TESTS; do
   printf "%-26s" "$t"
   for CXX in g++-16 clang++-22; do
      for STD in 03 11 17 20; do
         total=$((total+1)); log=$O/$t.$CXX.$STD.log
         if $CXX -std=c++$STD -O2 -DNDEBUG -Wall -Wextra -I$BR -I$EX $EX/segmented_${t}_test.cpp -o $O/t.elf > $log 2>&1; then
            if [ -s $log ]; then printf " %-8s" "WARN"; fails=$((fails+1)); bad="$bad $t/$CXX/$STD:warn"
            elif $O/t.elf >> $log 2>&1; then printf " %-8s" "pass"
            else printf " %-8s" "RUNFAIL"; fails=$((fails+1)); bad="$bad $t/$CXX/$STD:run"; fi
         else
            printf " %-8s" "BUILDERR"; fails=$((fails+1)); bad="$bad $t/$CXX/$STD:build"
         fi
      done
   done
   echo
done
echo "matrix: $total configurations, $fails not clean:$bad"

echo
echo "############ 4. sanitisers, C++20 -O1 -fsanitize=address,undefined ############"
for t in copy_if remove_copy remove_copy_if remove remove_if partition_copy copy; do
   for CXX in clang++-22 g++-16; do
      if $CXX -std=c++20 -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -Wall -Wextra -I$BR -I$EX $EX/segmented_${t}_test.cpp -o $O/s.elf > $O/san.$t.$CXX.log 2>&1; then
         if $O/s.elf >> $O/san.$t.$CXX.log 2>&1; then printf "%-18s %-12s clean\n" "$t" "$CXX"
         else printf "%-18s %-12s FAIL\n" "$t" "$CXX"; tail -15 $O/san.$t.$CXX.log | sed 's/^/    /'; fi
      else printf "%-18s %-12s BUILDERR\n" "$t" "$CXX"; head -10 $O/san.$t.$CXX.log | sed 's/^/    /'; fi
   done
done
for CXX in clang++-22 g++-16; do
   if $CXX -std=c++20 -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -Wall -Wextra -I$BR -I$EX $S/370_probe.cpp -o $O/sp.elf > $O/san.probe.$CXX.log 2>&1; then
      if $O/sp.elf > $O/san.probe.$CXX.out 2>>$O/san.probe.$CXX.log; then
         printf "%-18s %-12s clean, mismatches=%s\n" "370_probe" "$CXX" "$(grep -c MISMATCH $O/san.probe.$CXX.out)"
      else printf "%-18s %-12s FAIL\n" "370_probe" "$CXX"; tail -15 $O/san.probe.$CXX.log | sed 's/^/    /'; fi
   else printf "%-18s %-12s BUILDERR\n" "370_probe" "$CXX"; head -10 $O/san.probe.$CXX.log | sed 's/^/    /'; fi
done

echo
echo "############ 5. g22 depth survey ############"
D=$EX/cursor_build/g22/deep
NAMES=(x copy copy_if copy_n transform remove_copy remove_copy_if reverse_copy swap_ranges merge set_union set_difference set_intersection set_symmetric_difference partition_copy)
printf "%-26s %-8s %-8s\n" "algorithm" "depth1" "depth2"
allok=1
for i in $(seq 1 14); do
   res=()
   for d in 1 2; do
      if g++-16 -std=c++20 -O1 -DNDEBUG -DALGO=$i -DDEPTH=$d -I$BR -I$EX -c $D/202_survey.cpp -o $O/a.o 2>$O/e.$i.$d.log; then res+=("ok"); else res+=("FAIL"); allok=0; fi
   done
   printf "%-26s %-8s %-8s\n" "${NAMES[$i]}" "${res[0]}" "${res[1]}"
done
echo "depth survey all ok: $allok"
echo "DONE-FINAL"
