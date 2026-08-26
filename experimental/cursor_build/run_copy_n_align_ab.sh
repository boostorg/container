#!/bin/bash
set -eux
REPO=/mnt/d/Data/LocalGit/boost/libs/container
cd $REPO/experimental
INC="-I../../.. -I$REPO/include"
FLAGS="-std=c++20 -O3 -DNDEBUG -falign-functions=64 -falign-loops=64"

# Focused noinline probe with same align flags
g++-16 $FLAGS $INC \
  -o /tmp/cn_align $REPO/experimental/cursor_build/copy_n_std_nsg_probe.cpp
g++-16 $FLAGS $INC -S \
  -o /tmp/cn_align.s $REPO/experimental/cursor_build/copy_n_std_nsg_probe.cpp

echo "===== timings ====="
/tmp/cn_align
/tmp/cn_align

python3 - <<'PY'
from pathlib import Path
text = Path("/tmp/cn_align.s").read_text().splitlines()

def extract(name_substr):
    # find .globl line containing mangled do_std/do_nsg/do_seg
    start = None
    for i,l in enumerate(text):
        if l.startswith(".globl\t") and name_substr in l:
            # next non-directive label is the function; find from type line
            start = i
            break
    if start is None:
        # try function label containing the name after c++filt-style: search for _Z6do_
        for i,l in enumerate(text):
            if l.startswith("_Z6"+name_substr) or l.startswith("_Z6"+name_substr.replace("do_","")):
                start = i
                break
    # simpler: search for exact known prefixes
    return start

for tag, key in [("std","do_std"), ("nsg","do_nsg"), ("seg","do_seg")]:
    idx = None
    for i,l in enumerate(text):
        if l.startswith("_Z6"+key):
            idx = i
            break
    print(f"\n======== {tag} @{idx} ========")
    if idx is None:
        print("NOT FOUND")
        continue
    for j in range(idx, min(idx+80, len(text))):
        l = text[j]
        print(l)
        if j > idx and l.startswith("\t.size\t") and key in l:
            break
PY
