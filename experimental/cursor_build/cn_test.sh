#!/bin/bash
cd /mnt/d/Data/LocalGit/boost/libs/container/experimental || exit 1
for RA in 1 0; do
   echo "== RA=$RA =="
   g++ -std=c++17 -O2 -DNDEBUG \
      -DBOOST_CONTAINER_SEGMENTED_ENABLE_RA_SPECIALIZATIONS=$RA \
      -I../include -I/mnt/d/Data/LocalGit/boost \
      -o /tmp/scn_$RA segmented_copy_n_test.cpp 2>&1 | tail -n 20
   [ -x /tmp/scn_$RA ] && /tmp/scn_$RA
done
