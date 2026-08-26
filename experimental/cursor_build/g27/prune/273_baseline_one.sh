#!/bin/bash
ROOT=/mnt/d/Data/LocalGit/boost
EXP=$ROOT/libs/container/experimental
cd $ROOT/libs/container

echo "=== files modified in working tree ==="
git diff --name-only

echo
echo "=== baseline: original helper from HEAD, clang++-22 c++03 segmented_fill_test ==="
mkdir -p /tmp/base
git show HEAD:experimental/segmented_test_helper.hpp > /tmp/base/segmented_test_helper.hpp
clang++-22 -std=c++03 -O2 -DNDEBUG -Wall -Wextra \
   -I/tmp/base -I$ROOT -I$EXP -o /tmp/base/t.exe $EXP/segmented_fill_test.cpp 2>&1
echo "compile rc=$?"
echo "--- end baseline output ---"
