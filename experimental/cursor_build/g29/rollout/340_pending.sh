#!/bin/bash
# List unmodified segmented_*.hpp headers and any E-shape walker guards inside.
set -u
cd /mnt/d/Data/LocalGit/boost/libs/container
git diff --name-only -- include 2>/dev/null | sed 's|.*/||' > /tmp/mod.txt
echo "=== unmodified segmented headers ==="
for f in include/boost/container/experimental/segmented_*.hpp; do
   b=$(basename "$f")
   if ! grep -qx "$b" /tmp/mod.txt; then
      n=$(grep -c 'BOOST_LIKELY' "$f")
      echo "$b  SEG_LIKELY_hits=$n"
   fi
done
