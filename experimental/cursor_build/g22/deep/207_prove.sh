#!/bin/bash
BR=/mnt/d/Data/LocalGit/boost
cd $BR/libs/container/experimental || exit 1
g++-16 -std=c++20 -O2 -DNDEBUG -DBOOST_CONTAINER_PROVE_SEG_DST_REACHED \
   -include cstdlib -I$BR segmented_set_union_test.cpp -o /tmp/prv.elf 2>&1 | head -5
/tmp/prv.elf >/dev/null 2>&1
rc=$?
echo "exit=$rc"
if [ $rc -eq 134 ]; then
   echo "SIGABRT: the segmented-destination overload really executes under the test"
else
   echo "NOT reached: the test never instantiates/runs that overload"
fi
