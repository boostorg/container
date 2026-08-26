#!/bin/bash
# Shadow include tree holding *only* the four touched headers at their HEAD
# revision.  Put first on the include path it gives a clean A/B: everything
# else still resolves to the working tree.
set -e
REPO=/mnt/d/Data/LocalGit/boost/libs/container
PRE=/tmp/pf39pre
rm -rf $PRE
mkdir -p $PRE/boost/container/experimental
cd $REPO
for h in segmented_is_partitioned segmented_search segmented_search_n segmented_partition; do
  git show HEAD:include/boost/container/experimental/$h.hpp \
    > $PRE/boost/container/experimental/$h.hpp
done
ls -l $PRE/boost/container/experimental/
