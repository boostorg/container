#!/usr/bin/env python3
"""351: median-of-5 pinned numbers for the partition rows."""
import re, statistics, glob, os

OUT = "/mnt/d/Data/LocalGit/boost/libs/container/experimental/cursor_build/g30/nsgpart"

def parse(fn):
    rows = {}
    for line in open(fn):
        m = re.match(r"(partition\((?:hit|miss)\))\s+([\d.]+)\s+([\d.]+)\s+([\d.]+)\s+([\d.]+)\s+([\d.]+)\s+([\d.]+)", line)
        if m:
            rows[m.group(1)] = tuple(float(x) for x in m.groups()[1:])
    return rows

for cc in ("gcc", "clang"):
    files = sorted(glob.glob(os.path.join(OUT, f"350_{cc}_pin*.txt")))
    per_row = {}
    for fn in files:
        for k, v in parse(fn).items():
            per_row.setdefault(k, []).append(v)
    print(f"== {cc} pinned median of {len(files)} (seg / std / nsg ns per elem, then all runs) ==")
    for k, vs in sorted(per_row.items()):
        segs = [v[3] for v in vs]; stds = [v[4] for v in vs]; nsgs = [v[5] for v in vs]
        med = (statistics.median(segs), statistics.median(stds), statistics.median(nsgs))
        print(f"{k:20s} seg={med[0]:.3f} std={med[1]:.3f} nsg={med[2]:.3f}  nsg/std={med[2]/med[1]:.2f}")
        print(f"{'':20s} seg runs {segs}")
        print(f"{'':20s} std runs {stds}")
        print(f"{'':20s} nsg runs {nsgs}")
