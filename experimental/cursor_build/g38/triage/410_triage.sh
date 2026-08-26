#!/bin/bash
set -u
BR=/mnt/d/Data/LocalGit/boost
EX=$BR/libs/container/experimental
O=/tmp/g38
mkdir -p $O
cd $EX || exit 1

pass=0; runfail=0; buildfail=0
> $O/fails.txt
for f in segmented_*_test.cpp; do
   t=${f%.cpp}
   if g++-16 -std=c++20 -O2 -DNDEBUG -I$BR -I$EX -Wall -Wextra "$f" -o $O/$t.elf 2>$O/$t.blog; then
      if [ -s $O/$t.blog ]; then
         echo "WARN-OUTPUT  $t" >> $O/fails.txt
      fi
      if $O/$t.elf > $O/$t.rlog 2>&1; then
         pass=$((pass+1))
      else
         runfail=$((runfail+1))
         n=$(grep -c -iE 'failed|error' $O/$t.rlog)
         echo "RUN-FAIL     $t   (exit=$? reported=$n)" >> $O/fails.txt
         echo "   first: $(grep -m1 -iE 'failed|error' $O/$t.rlog | cut -c1-160)" >> $O/fails.txt
      fi
   else
      buildfail=$((buildfail+1))
      echo "BUILD-FAIL   $t" >> $O/fails.txt
      grep -m3 'error:' $O/$t.blog | sed 's/^/   /' >> $O/fails.txt
   fi
done

echo "g++-16 -std=c++20 : pass=$pass run-fail=$runfail build-fail=$buildfail  (total $(ls segmented_*_test.cpp | wc -l))"
echo
cat $O/fails.txt
