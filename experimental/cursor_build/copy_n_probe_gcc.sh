#!/bin/bash
# Focused A/B probe: HEAD segmented_copy_n.hpp (BASE) vs working tree (NEW).
set -u
REPO=/mnt/d/Data/LocalGit/boost/libs/container
CXX=${CXX:-g++}
TAG=${TAG:-gcc}
HDR=include/boost/container/experimental/segmented_copy_n.hpp
WORK=$REPO/experimental/cursor_build/cn_ab
SHADOW=$WORK/base_inc/boost/container/experimental

mkdir -p $SHADOW
( cd $REPO && git show HEAD:$HDR ) > $SHADOW/segmented_copy_n.hpp || exit 1

INC="-I$REPO/include -I/mnt/d/Data/LocalGit/boost"
FLAGS="-std=c++20 -O3 -DNDEBUG"
SRC=$REPO/experimental/cursor_build/copy_n_probe.cpp

$CXX $FLAGS -DCOPY_N_BASE -I$WORK/base_inc $INC -o /tmp/cnp_base_$TAG $SRC || exit 1
$CXX $FLAGS                                $INC -o /tmp/cnp_new_$TAG  $SRC || exit 1

for R in 1 2; do
   /tmp/cnp_base_$TAG
   /tmp/cnp_new_$TAG
done
