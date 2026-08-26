#!/bin/bash
# Per-symbol size listing (text/weak) for the requested variant builds.
cd /mnt/d/Data/LocalGit/boost/libs/container/experimental/cursor_build/g44/findend/out/codegen || exit 1
for f in "$@"; do
  echo "--- $f"
  nm -C --size-sort -S "$f.o" | grep -iE ' (t|w) '
done
