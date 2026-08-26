#!/bin/bash
# Full gate: (1) all segmented tests x {g++-16, clang++-22} x {c++03,11,17,20}
# -O2 -DNDEBUG -Wall -Wextra, silent + passing; (2) ASan+UBSan -O1 c++20 on the
# recursive-coverage set; (3) g22 depth survey; (4) summed .text after vs the
# bin_before baseline.
set -u
BR=/mnt/d/Data/LocalGit/boost
EX=$BR/libs/container/experimental
G22=$BR/libs/container/experimental/cursor_build/g22/deep
BB=$BR/libs/container/experimental/cursor_build/g29/rollout/bin_before
export BR EX
export O=/tmp/g29F
rm -rf $O; mkdir -p $O

echo "=== 1. full matrix ==="
for T in $EX/segmented_*_test.cpp; do
   B=$(basename $T .cpp)
   for CC in g++-16 clang++-22; do
      for STD in c++03 c++11 c++17 c++20; do
         echo "$B $CC $STD"
      done
   done
done > $O/jobs.txt

xargs -P 6 -n 3 -a $O/jobs.txt bash -c '
   B=$0; CC=$1; STD=$2
   log=$O/$B.$CC.$STD.log
   bin=$O/$B.$CC.$STD
   res=$O/res.$B.$CC.$STD
   if ! $CC -std=$STD -O2 -DNDEBUG -Wall -Wextra -I$BR -I$EX $EX/$B.cpp -o $bin >$log 2>&1; then
      echo "BUILD-FAIL" > $res; exit 0
   fi
   if [ -s $log ]; then
      if [ "$B" = "segmented_fill_test" ] && [ "$CC" = "clang++-22" ] && [ "$STD" = "c++03" ] \
         && [ "$(grep -c "warning:" $log)" = "1" ] && grep -q "C++11 extension" $log; then
         : # known pre-existing exception
      else
         echo "WARN" > $res; exit 0
      fi
   fi
   if $bin >$log.run 2>&1; then echo "pass" > $res; else echo "RUNFAIL" > $res; fi
'

mfails=0
for r in $O/res.*; do
   s=$(cat $r)
   if [ "$s" != "pass" ]; then
      echo "  ${r##*/res.}: $s"
      base=${r##*/res.}
      head -15 $O/$base.log 2>/dev/null | sed 's/^/      /'
      mfails=$((mfails+1))
   fi
done
total=$(ls $O/res.* | wc -l)
echo "  matrix: $((total-mfails))/$total pass"

echo
echo "=== 2. sanitizers (ASan+UBSan, -O1, c++20) ==="
sfails=0
for A in set_union set_difference set_intersection set_symmetric_difference \
         merge copy copy_if fill find count reverse partition_copy; do
   for CC in g++-16 clang++-22; do
      log=$O/san.$A.$CC.log
      if ! $CC -std=c++20 -O1 -fsanitize=address,undefined -I$BR -I$EX \
           $EX/segmented_${A}_test.cpp -o $O/san.$A.$CC >$log 2>&1; then
         echo "  $A $CC SAN-BUILD-FAIL"; sfails=1; head -10 $log | sed 's/^/      /'; continue
      fi
      if $O/san.$A.$CC >$log.run 2>&1 && ! grep -qE 'ERROR|runtime error' $log.run; then
         echo "  $A $CC clean"
      else
         echo "  $A $CC SAN-FAIL"; sfails=1; tail -20 $log.run | sed 's/^/      /'
      fi
   done
done

echo
echo "=== 3. depth survey ==="
tr -d '\r' < $G22/203_survey.sh > /tmp/s203.sh && bash /tmp/s203.sh

echo
echo "=== 4. .text totals (c++20 -O2) before vs after ==="
for CC in g++-16 clang++-22; do
   tot=0
   for bin in $O/segmented_*_test.$CC.c++20; do
      t=$(size -A "$bin" | awk '$1==".text"{print $2}')
      tot=$((tot + t))
   done
   echo "  $CC after total .text: $tot"
done
cat $BB/text_sizes.txt | sed 's/^/  before: /'

echo
[ $mfails -eq 0 ] && [ $sfails -eq 0 ] && echo "305: MATRIX+SAN OK" || echo "305: FAILURES PRESENT"
