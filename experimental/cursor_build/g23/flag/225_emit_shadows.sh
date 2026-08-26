#!/bin/bash
# Writes the recommended set_* shadow headers into the g23 directory itself so
# MSVC (which cannot see WSL's /tmp) can be pointed at them with /I.
set -u
G=/tmp/g23
OUT=/mnt/d/Data/LocalGit/boost/libs/container/experimental/cursor_build/g23/flag/shadow_guard
rm -rf $OUT
for A in set_union set_difference set_intersection set_symmetric_difference; do
   python3 $G/220_mkshadow_sets.py $A guard \
      $OUT/boost/container/experimental/segmented_$A.hpp || exit 1
done
ls -l $OUT/boost/container/experimental/
