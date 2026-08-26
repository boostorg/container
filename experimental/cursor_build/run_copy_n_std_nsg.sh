#!/bin/bash
set -eux
REPO=/mnt/d/Data/LocalGit/boost/libs/container
SRC=$REPO/experimental/cursor_build/copy_n_std_nsg_probe.cpp
OUT=$REPO/experimental/cursor_build/cn_std_nsg
INC="-I$REPO/include -I/mnt/d/Data/LocalGit/boost"
g++-16 --version | head -1
g++-16 -std=c++20 -O3 -DNDEBUG $INC -S -o ${OUT}.s $SRC
g++-16 -std=c++20 -O3 -DNDEBUG $INC -o ${OUT} $SRC
${OUT}
${OUT}
grep -n "do_std\|do_nsg\|do_seg" ${OUT}.s | head -20
