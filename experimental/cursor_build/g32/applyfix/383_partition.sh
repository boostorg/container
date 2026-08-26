#!/bin/bash
set -u
BR=/mnt/d/Data/LocalGit/boost
EX=$BR/libs/container/experimental
S=$EX/cursor_build/g32/applyfix
SH=$S/pre
O=/tmp/g32part
mkdir -p $O

tr -d '\r' < $S/377_mkpre.py > /tmp/mkpre383.py
python3 /tmp/mkpre383.py || exit 1

echo "=== segmented_partition_test with the four leaves REVERTED (pre) ==="
g++-16 -std=c++20 -O2 -DNDEBUG -Wall -Wextra -I$SH -I$BR -I$EX $EX/segmented_partition_test.cpp -o $O/pre.elf || echo BUILDERR
$O/pre.elf; echo "pre rc=$?"

echo
echo "=== segmented_partition_test against the current tree (post) ==="
g++-16 -std=c++20 -O2 -DNDEBUG -Wall -Wextra -I$BR -I$EX $EX/segmented_partition_test.cpp -o $O/post.elf || echo BUILDERR
$O/post.elf; echo "post rc=$?"

echo
echo "=== same test at the last commit before the fix (6a1a7d6^) ==="
GIT=$BR/libs/container
mkdir -p $O/base/boost/container/experimental
cd $GIT
for f in $(git ls-tree --name-only -r 6a1a7d6^ include/boost/container/experimental | sed 's|.*/||'); do
   git show 6a1a7d6^:include/boost/container/experimental/$f > $O/base/boost/container/experimental/$f
done
g++-16 -std=c++20 -O2 -DNDEBUG -Wall -Wextra -I$O/base -I$BR -I$EX $EX/segmented_partition_test.cpp -o $O/base.elf || echo BUILDERR
$O/base.elf; echo "base rc=$?"
echo DONE-PART
