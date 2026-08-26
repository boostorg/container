#!/bin/bash
# Does the depth-2 destination coverage actually have teeth?  Break the new
# walker (skip the "restart at the beginning of the last segment" step) and
# confirm the test notices.  The header is restored afterwards either way.
BR=/mnt/d/Data/LocalGit/boost
H=$BR/boost/container/experimental/segmented_set_union.hpp
EX=$BR/libs/container/experimental
cp $H /tmp/su.orig || exit 1
restore() { cp /tmp/su.orig $H; }
trap restore EXIT

sed -i 's|^      db = dst_traits::begin(slast); //MUTATION-TEST-ANCHOR$|      /*mutated: db not restarted*/|' $H
if ! grep -q 'mutated: db not restarted' $H; then
   echo "could not apply mutation"; exit 1
fi
echo "mutation applied:"
grep -n 'mutated: db not restarted' $H

cd $EX || exit 1
if g++-16 -std=c++20 -O2 -DNDEBUG -I$BR segmented_set_union_test.cpp -o /tmp/mut.elf 2>/tmp/mut.log; then
   /tmp/mut.elf >/tmp/mut.out 2>&1
   rc=$?
   echo "test exit=$rc"
   if [ $rc -ne 0 ]; then
      echo "GOOD: the mutation is caught by the depth-2 destination coverage"
      grep -cE 'test.*failed|error' /tmp/mut.out | sed 's/^/   failing assertions: /'
   else
      echo "BAD: mutation went unnoticed -- the new path is not really asserted on"
   fi
else
   echo "build failed"; grep -m3 'error:' /tmp/mut.log
fi
