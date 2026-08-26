#!/bin/bash
set -eux
REPO=/mnt/d/Data/LocalGit/boost/libs/container
SRC=$REPO/experimental/cursor_build/copy_n_std_nsg_probe.cpp
OUT=$REPO/experimental/cursor_build
INC="-I$REPO/include -I/mnt/d/Data/LocalGit/boost"

# Correctness
g++-16 -std=c++20 -O2 -DNDEBUG $INC \
  -o /tmp/scn_test $REPO/experimental/segmented_copy_n_test.cpp
/tmp/scn_test

# A/B timings + asm for nsg leaf
for FORM in 0 1; do
  g++-16 -std=c++20 -O3 -DNDEBUG \
    -DBOOST_CONTAINER_SEGMENTED_COPY_N_LIBSTDCPP_LOOP=$FORM \
    $INC -S -o $OUT/cn_loop_$FORM.s $SRC
  g++-16 -std=c++20 -O3 -DNDEBUG \
    -DBOOST_CONTAINER_SEGMENTED_COPY_N_LIBSTDCPP_LOOP=$FORM \
    $INC -o /tmp/cn_loop_$FORM $SRC
  echo "===== LOOP=$FORM ====="
  /tmp/cn_loop_$FORM
  /tmp/cn_loop_$FORM
done

echo "===== nsg hot loop LOOP=0 ====="
# extract do_nsg body roughly
awk '/do_nsg/,/\.size.*do_nsg/' $OUT/cn_loop_0.s | head -80
echo "===== nsg hot loop LOOP=1 ====="
awk '/do_nsg/,/\.size.*do_nsg/' $OUT/cn_loop_1.s | head -80
