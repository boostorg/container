#!/bin/bash
ROOT=/mnt/d/Data/LocalGit/boost
EXP=$ROOT/libs/container/experimental
cd $ROOT/libs/container
echo "HEAD version lines:        $(git show HEAD:experimental/segmented_test_helper.hpp | wc -l)"
echo "working tree (pruned):     $(wc -l < $EXP/segmented_test_helper.hpp)"
echo
echo "=== diff of helper vs HEAD (unified, code only view) ==="
git diff --stat -- experimental/segmented_test_helper.hpp
