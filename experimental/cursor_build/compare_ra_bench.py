#!/usr/bin/env python3
"""Compare RA=0 vs RA=1 bench_segmented_algos group-25 outputs."""
from __future__ import annotations
import re
from pathlib import Path

OUT = Path(__file__).with_name('ra_bench')

# line: name ratios... seg_ns std_ns nsg_ns
ROW_RE = re.compile(
    r'^(?P<name>\S.*?)\s+'
    r'(?P<nsg_seg>[-+]?\d+\.\d+)\s+'
    r'(?P<std_seg>[-+]?\d+\.\d+)\s+'
    r'(?P<std_nsg>[-+]?\d+\.\d+)\s+'
    r'(?P<seg>[-+]?\d+\.\d+)\s+'
    r'(?P<std>[-+]?\d+\.\d+)\s+'
    r'(?P<nsg>[-+]?\d+\.\d+)\s*$'
)
HEADER_RE = re.compile(r'^=== Segmented algorithm benchmark \[(?P<vt>.+)\] ===')
GROUP_GEO_RE = re.compile(r'^group geomean\s+(?P<nsg_seg>[-+]?\d+\.\d+)\s+(?P<std_seg>[-+]?\d+\.\d+)\s+(?P<std_nsg>[-+]?\d+\.\d+)')
ALGO_GEO_RE = re.compile(r'^algo geomean\s+(?P<nsg_seg>[-+]?\d+\.\d+)\s+(?P<std_seg>[-+]?\d+\.\d+)\s+(?P<std_nsg>[-+]?\d+\.\d+)')

SKIP_NAMES = {'group geomean', 'algo geomean'}


def parse(path: Path):
    text = path.read_text(encoding='utf-8', errors='replace')
    cur_vt = None
    rows = {}  # (vt, name) -> dict
    geos = {}  # (vt, kind) -> dict
    for line in text.splitlines():
        m = HEADER_RE.match(line.strip())
        if m:
            cur_vt = m.group('vt')
            continue
        if cur_vt is None:
            continue
        m = GROUP_GEO_RE.match(line.strip())
        if m:
            geos[(cur_vt, 'group')] = {k: float(m.group(k)) for k in ('nsg_seg', 'std_seg', 'std_nsg')}
            # also capture trailing seg ns if present: " [type]"
            continue
        m = ALGO_GEO_RE.match(line.strip())
        if m:
            geos[(cur_vt, 'algo')] = {k: float(m.group(k)) for k in ('nsg_seg', 'std_seg', 'std_nsg')}
            continue
        m = ROW_RE.match(line.rstrip())
        if not m:
            continue
        name = m.group('name').strip()
        if name in SKIP_NAMES or name.startswith('<') or name.startswith('-'):
            continue
        rows[(cur_vt, name)] = {k: float(m.group(k)) for k in ('nsg_seg', 'std_seg', 'std_nsg', 'seg', 'std', 'nsg')}
    return rows, geos


def shorten_vt(vt: str) -> str:
    if 'MyInt' in vt and 'Fat' not in vt:
        return 'MyInt'
    if 'MyFatInt' in vt:
        if 'Lm4' in vt or '4>' in vt or 'ILm4' in vt:
            return 'Fat4'
        if 'Lm8' in vt or '8>' in vt or 'ILm8' in vt:
            return 'Fat8'
    return vt[:16]


def compare(tag: str):
    p0 = OUT / f'{tag}_ra0.txt'
    p1 = OUT / f'{tag}_ra1.txt'
    if not p0.exists() or not p1.exists():
        print(f'## {tag}: missing files')
        return
    r0, g0 = parse(p0)
    r1, g1 = parse(p1)
    keys = sorted(set(r0) | set(r1), key=lambda k: (shorten_vt(k[0]), k[1]))
    print(f'\n## {tag}: RA1/RA0 on seg ns (lower is better for RA1; >1 means RA1 slower)')
    print(f'{"vt":6} {"algo":32} {"seg0":>8} {"seg1":>8} {"1/0":>7} {"std0":>8} {"std1":>8} {"std/seg0":>8} {"std/seg1":>8}')
    notable = []
    for vt, name in keys:
        if (vt, name) not in r0 or (vt, name) not in r1:
            continue
        a, b = r0[(vt, name)], r1[(vt, name)]
        ratio = b['seg'] / a['seg'] if a['seg'] else float('nan')
        sv = shorten_vt(vt)
        print(f'{sv:6} {name:32} {a["seg"]:8.3f} {b["seg"]:8.3f} {ratio:7.3f} {a["std"]:8.3f} {b["std"]:8.3f} {a["std_seg"]:8.2f} {b["std_seg"]:8.2f}')
        if abs(ratio - 1.0) >= 0.05:  # >=5% change
            notable.append((abs(ratio - 1.0), sv, name, a['seg'], b['seg'], ratio, a['std_seg'], b['std_seg']))
    print(f'\n### {tag}: notable seg changes (>=5%)')
    for _, sv, name, s0, s1, ratio, ss0, ss1 in sorted(notable, reverse=True):
        direction = 'FASTER' if ratio < 1 else 'SLOWER'
        print(f'  {sv:6} {name:32} {s0:7.3f}->{s1:7.3f} ({ratio:.3f}x, {direction})  std/seg {ss0:.2f}->{ss1:.2f}')

    print(f'\n### {tag}: group geomean std/seg')
    for vt, kind in sorted(set(g0) | set(g1)):
        if kind != 'group':
            continue
        if (vt, kind) not in g0 or (vt, kind) not in g1:
            continue
        a, b = g0[(vt, kind)], g1[(vt, kind)]
        print(f'  {shorten_vt(vt):6} std/seg {a["std_seg"]:.3f} -> {b["std_seg"]:.3f}  (delta {b["std_seg"]-a["std_seg"]:+.3f})')


def main():
    for tag in ('gcc16', 'clang22', 'msvc145'):
        compare(tag)


if __name__ == '__main__':
    main()
