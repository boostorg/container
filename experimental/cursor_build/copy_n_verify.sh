#!/bin/bash
# Correctness matrix for segmented_copy_n, then the focused A/B probe.
set -u
REPO=/mnt/d/Data/LocalGit/boost/libs/container
INC="-I$REPO/include -I/mnt/d/Data/LocalGit/boost"
rc=0
for RA in 1 0; do
   for STD in 11 20; do
      OUT=/tmp/scn_${RA}_${STD}
      g++ -std=c++$STD -O2 -DNDEBUG \
         -DBOOST_CONTAINER_SEGMENTED_ENABLE_RA_SPECIALIZATIONS=$RA \
         $INC -o $OUT $REPO/experimental/segmented_copy_n_test.cpp 2>&1 | tail -n 20
      if [ ! -x $OUT ]; then echo "RA=$RA std=$STD BUILD FAILED"; rc=1; continue; fi
      if $OUT >/dev/null; then echo "RA=$RA std=$STD OK"; else echo "RA=$RA std=$STD FAILED"; rc=1; fi
   done
done
exit $rc
