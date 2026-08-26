#!/bin/bash
# The exact recommended patch for the four set_* headers, as a unified diff of
# the persisted shadow against the real header.
set -u
BR=/mnt/d/Data/LocalGit/boost
SH=$BR/libs/container/experimental/cursor_build/g23/flag/shadow_guard
for A in set_union set_difference set_intersection set_symmetric_difference; do
   R=$BR/boost/container/experimental/segmented_$A.hpp
   echo "############################## segmented_$A.hpp"
   diff -u --label a/segmented_$A.hpp --label b/segmented_$A.hpp \
        <(tr -d '\r' < $R) $SH/boost/container/experimental/segmented_$A.hpp
done
