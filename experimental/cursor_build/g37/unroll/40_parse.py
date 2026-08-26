#!/usr/bin/env python3
"""Parse bench_segmented_algos group tables into per-(type,row) medians and
print before/after comparisons between variants."""
import glob, os, re, sys, math
from statistics import median

R = "/mnt/d/Data/LocalGit/boost/libs/container/experimental/cursor_build/g37/unroll/runs"

TYPE_RE = re.compile(r"=== Segmented algorithm benchmark \[(.+?)\] ===")
ROW_RE = re.compile(r"^(\S.*?)\s{2,}([\d.]+)\s+([\d.]+)\s+([\d.]+)\s+([\d.]+)\s+([\d.]+)\s+([\d.]+)\s*$")

PRETTY = {"i": "int", "5MyInt": "MyInt",
          "8MyFatIntILm2EE": "MyFatInt<2>", "8MyFatIntILm8EE": "MyFatInt<8>"}


def parse(path):
    """-> {(type,row): (nsg_seg, std_seg, std_nsg, seg, std, nsg)}"""
    out = {}
    cur = None
    for line in open(path, errors="replace"):
        m = TYPE_RE.search(line)
        if m:
            cur = PRETTY.get(m.group(1), m.group(1))
            continue
        if cur is None or line.startswith("<") or "geomean" in line:
            continue
        m = ROW_RE.match(line.rstrip("\n"))
        if m:
            out[(cur, m.group(1).strip())] = tuple(float(m.group(i)) for i in range(2, 8))
    return out


def collect(cc, variant, group):
    files = sorted(glob.glob(os.path.join(R, f"{cc}_{variant}_g{group}_r*.txt")))
    if not files:
        return None, 0
    runs = [parse(f) for f in files]
    keys = set(runs[0])
    for r in runs[1:]:
        keys &= set(r)
    agg = {}
    for k in keys:
        cols = []
        for i in range(6):
            vals = [r[k][i] for r in runs]
            cols.append((median(vals), min(vals), max(vals)))
        agg[k] = cols
    return agg, len(files)


def geo(vals):
    vals = [v for v in vals if v > 0]
    return math.exp(sum(math.log(v) for v in vals) / len(vals)) if vals else float("nan")


def main():
    group = sys.argv[1] if len(sys.argv) > 1 else "25"
    base_v = sys.argv[2] if len(sys.argv) > 2 else "base"
    variants = sys.argv[3:] or ["p2_copyif"]
    rowfilter = os.environ.get("ROWS", "")

    for cc in ("g++-16", "clang++-22"):
        base, nb = collect(cc, base_v, group)
        if base is None:
            continue
        for v in variants:
            cmp_, nc = collect(cc, v, group)
            if cmp_ is None:
                continue
            print(f"\n=== {cc}: {base_v}({nb} runs) -> {v}({nc} runs), group {group} ===")
            types = sorted({t for (t, _) in base}, key=lambda s: list(PRETTY.values()).index(s)
                           if s in PRETTY.values() else 99)
            for t in types:
                rows = [r for (tt, r) in base if tt == t and (tt, r) in cmp_]
                if rowfilter:
                    rows = [r for r in rows if re.search(rowfilter, r)]
                if not rows:
                    continue
                rows.sort()
                print(f"\n-- {t} --")
                print(f"{'row':<26} {'seg b':>8} {'seg a':>8} {'d%':>7} "
                      f"{'nsg b':>8} {'nsg a':>8} {'d%':>7} {'std/nsg b':>9} {'std/nsg a':>9} {'spr%':>6}")
                for r in rows:
                    b, c = base[(t, r)], cmp_[(t, r)]
                    segb, sega = b[3][0], c[3][0]
                    nsgb, nsga = b[5][0], c[5][0]
                    sn_b, sn_a = b[2][0], c[2][0]
                    spr = 100.0 * (c[5][2] - c[5][1]) / c[5][0] if c[5][0] else 0
                    print(f"{r:<26} {segb:8.3f} {sega:8.3f} {100*(sega/segb-1):+7.1f} "
                          f"{nsgb:8.3f} {nsga:8.3f} {100*(nsga/nsgb-1):+7.1f} "
                          f"{sn_b:9.2f} {sn_a:9.2f} {spr:6.1f}")
                gsb = geo([base[(t, r)][3][0] for r in rows])
                gsa = geo([cmp_[(t, r)][3][0] for r in rows])
                gnb = geo([base[(t, r)][5][0] for r in rows])
                gna = geo([cmp_[(t, r)][5][0] for r in rows])
                print(f"{'GEOMEAN(' + str(len(rows)) + ' rows)':<26} {gsb:8.4f} {gsa:8.4f} "
                      f"{100*(gsa/gsb-1):+7.1f} {gnb:8.4f} {gna:8.4f} {100*(gna/gnb-1):+7.1f}")


main()
