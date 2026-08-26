#!/bin/bash
# EOL convention of the files we are about to write, and the size of the other
# agent's in-tree changes, so the patch can be applied without an EOL flip.
set -u
BR=/mnt/d/Data/LocalGit/boost
cd $BR/boost/container/experimental
for f in segmented_set_union.hpp segmented_set_difference.hpp \
         segmented_set_intersection.hpp segmented_set_symmetric_difference.hpp \
         segmented_merge.hpp segmented_copy.hpp; do
   tot=$(wc -l "$f" | awk '{print $1}')
   cr=$(tr -dc '\r' < "$f" | wc -c)
   printf '  %-45s lines=%-5s CR=%s\n' "$f" "$tot" "$cr"
done
echo '--- helper ---'
f=$BR/libs/container/experimental/segmented_test_helper.hpp
printf '  %-45s lines=%-5s CR=%s\n' segmented_test_helper.hpp \
   "$(wc -l "$f" | awk '{print $1}')" "$(tr -dc '\r' < "$f" | wc -c)"
echo '--- other agent'"'"'s in-tree diff sizes (vs HEAD) ---'
cd $BR/libs/container
git diff --stat -- include/boost/container/experimental/segmented_set_union.hpp \
   include/boost/container/experimental/segmented_set_difference.hpp \
   include/boost/container/experimental/segmented_set_intersection.hpp \
   include/boost/container/experimental/segmented_set_symmetric_difference.hpp \
   include/boost/container/experimental/segmented_merge.hpp \
   experimental/segmented_test_helper.hpp
