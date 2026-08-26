#!/usr/bin/env python3
"""358: median-of-5 pinned partition rows for a given file prefix (350/356/357...)."""
import re, statistics, glob, os, sys

OUT = "/mnt/d/Data/LocalGit/boost/libs/container/experimental/cursor_build/g30/nsgpart"
prefix = sys.argv[1] if len(sys.argv) > 1 else "350"

def parse(fn):
    rows = {}
    for line in open(fn):
        m = re.match(r"(partition\((?:hit|miss)\))\s+([\d.]+)\s+([\d.]+)\s+([\d.]+)\s+([\d.]+)\s+([\d.]+)\s+([\d.]+)", line)
        if m:
            rows[m.group(1)] = tuple(float(x) for x in m.groups()[1:])
    return rows

for cc in ("gcc", "clang"):
    files = sorted(glob.glob(os.path.join(OUT, f"{prefix}_{cc}_pin*.txt")))
    per_row = {}
    for fn in files:
        for k, v in parse(fn).items():
            per_row.setdefault(k, []).append(v)
    print(f"== {prefix} {cc} pinned medians ({len(files)} runs) ==")
    for k, vs in sorted(per_row.items()):
        med = tuple(statistics.median([v[i] for v in vs]) for i in (3, 4, 5))
        print(f"{k:18s} seg={med[0]:.3f} std={med[1]:.3f} nsg={med[2]:.3f}  nsg/std={med[2]/med[1]:.2f}"
              f"   nsg runs {[v[5] for v in vs]}")
