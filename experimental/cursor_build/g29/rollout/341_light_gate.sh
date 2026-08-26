#!/bin/bash
# Reduced verification pass (user instruction): build+run ALL segmented tests
# once per compiler, c++20 only, -O2 -DNDEBUG -Wall -Wextra, zero warnings.
set -u
BR=/mnt/d/Data/LocalGit/boost
EX=$BR/libs/container/experimental
export BR EX
export O=/tmp/g29L
rm -rf $O; mkdir -p $O

for T in $EX/segmented_*_test.cpp; do
   B=$(basename $T .cpp)
   for CC in g++-16 clang++-22; do
      echo "$B $CC"
   done
done > $O/jobs.txt

xargs -P 6 -n 2 -a $O/jobs.txt bash -c '
   B=$0; CC=$1
   log=$O/$B.$CC.log
   if ! $CC -std=c++20 -O2 -DNDEBUG -Wall -Wextra -I$BR -I$EX $EX/$B.cpp -o $O/$B.$CC >$log 2>&1; then
      echo "BUILD-FAIL" > $O/res.$B.$CC; exit 0
   fi
   if [ -s $log ]; then echo "WARN" > $O/res.$B.$CC; exit 0; fi
   if $O/$B.$CC >$log.run 2>&1; then echo "pass" > $O/res.$B.$CC
   else echo "RUNFAIL" > $O/res.$B.$CC; fi
'

fails=0
for r in $O/res.*; do
   s=$(cat $r)
   if [ "$s" != "pass" ]; then
      echo "  ${r##*/res.}: $s"
      head -20 $O/${r##*/res.}.log 2>/dev/null | sed 's/^/      /'
      tail -10 $O/${r##*/res.}.log.run 2>/dev/null | sed 's/^/      /'
      fails=$((fails+1))
   fi
done
total=$(ls $O/res.* | wc -l)
echo "341: $((total-fails))/$total pass (g++-16 + clang++-22, c++20)"
