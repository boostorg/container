#!/usr/bin/env python3
"""Generate tables (markdown) and charts (png) for the article
"Neoclassical C++ (2): Exploring input-output segmented algorithms"
from the ra_bench logs (gcc16/clang22/msvc145 x ra0/ra1)."""
from __future__ import annotations
import math
import re
from pathlib import Path

import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt

HERE = Path(__file__).parent
RA = HERE / 'ra_bench'
OUT = RA / 'article'
OUT.mkdir(exist_ok=True)

ROW_RE = re.compile(
    r'^(?P<name>\S.*?)\s+'
    r'(?P<nsg_seg>[-+]?\d+\.\d+)\s+(?P<std_seg>[-+]?\d+\.\d+)\s+(?P<std_nsg>[-+]?\d+\.\d+)\s+'
    r'(?P<seg>[-+]?\d+\.\d+)\s+(?P<std>[-+]?\d+\.\d+)\s+(?P<nsg>[-+]?\d+\.\d+)\s*$')
HEADER_RE = re.compile(r'^=== Segmented algorithm benchmark \[(?P<vt>.+)\] ===')

COMPILERS = [('gcc16', 'GCC 16'), ('clang22', 'Clang 22'), ('msvc145', 'MSVC 2026')]
TYPES = ['MyInt', 'Fat4', 'Fat8']
TYPE_LABEL = {'MyInt': 'MyInt', 'Fat4': 'MyFatInt<4>', 'Fat8': 'MyFatInt<8>'}
TYPE_SUFFIX = {'MyInt': 'myint', 'Fat4': 'myfatint4', 'Fat8': 'myfatint8'}
SHAPES = ['1S', '2S', '1+2S']
SHAPE_SUFFIX = {'1S': '1s', '2S': '2s', '1+2S': '1p2s'}
SHAPE_LABEL = {
    '1S': '1S (segmented input, flat output)',
    '2S': '2S (flat input, segmented output)',
    '1+2S': '1+2S (segmented input and output)',
}
RATIOS = ['nsg_seg', 'std_seg', 'std_nsg']
RATIO_LABEL = {'nsg_seg': 'nsg/seg', 'std_seg': 'std/seg', 'std_nsg': 'std/nsg'}

# Algorithm order as printed by the benchmark
ALGOS = [
    'copy(1S)', 'copy(2S)', 'copy(1+2S)',
    'copy_if(1S hit)', 'copy_if(2S hit)', 'copy_if(1+2S hit)',
    'copy_if(1S miss)', 'copy_if(2S miss)', 'copy_if(1+2S miss)',
    'copy_n(1S)', 'copy_n(2S)', 'copy_n(1+2S)',
    'remove_copy(1S hit)', 'remove_copy(2S hit)', 'remove_copy(1+2S hit)',
    'remove_copy(1S miss)', 'remove_copy(2S miss)', 'remove_copy(1+2S miss)',
    'remove_copy_if(1S hit)', 'remove_copy_if(2S hit)', 'remove_copy_if(1+2S hit)',
    'remove_copy_if(1S miss)', 'remove_copy_if(2S miss)', 'remove_copy_if(1+2S miss)',
    'swap_ranges(1S)', 'swap_ranges(2S)', 'swap_ranges(1+2S)',
    'transform(1S)', 'transform(2S)', 'transform(1+2S)',
]


def algo_shape(name: str) -> str:
    if '(1+2S' in name:
        return '1+2S'
    if '(2S' in name:
        return '2S'
    if '(1S' in name:
        return '1S'
    raise ValueError(name)


def algos_for(shape: str) -> list[str]:
    return [a for a in ALGOS if algo_shape(a) == shape]


def algo_label(name: str) -> str:
    """Strip the shape token for shorter chart / table labels."""
    for shape in ('1+2S', '2S', '1S'):
        name = name.replace(f'({shape} ', '(').replace(f'({shape})', '')
    return name.strip() or name


def shorten_vt(vt: str) -> str:
    if 'MyInt' in vt and 'Fat' not in vt:
        return 'MyInt'
    if 'FatInt' in vt:
        return 'Fat4' if ('Lm4' in vt or '4>' in vt) else 'Fat8'
    return vt


def parse(path: Path) -> dict:
    """-> {(vt, algo): {ratio: float, seg/std/nsg: float}}"""
    rows = {}
    cur = None
    for line in path.read_text(encoding='utf-8', errors='replace').splitlines():
        m = HEADER_RE.match(line.strip())
        if m:
            cur = shorten_vt(m.group('vt'))
            continue
        m = ROW_RE.match(line.rstrip())
        if not m or cur is None:
            continue
        name = m.group('name').strip()
        if name in ALGOS:
            rows[(cur, name)] = {k: float(m.group(k)) for k in
                                 ('nsg_seg', 'std_seg', 'std_nsg', 'seg', 'std', 'nsg')}
    return rows


def geomean(vals):
    vals = [v for v in vals if v > 0]
    return math.exp(sum(math.log(v) for v in vals) / len(vals)) if vals else float('nan')


DATA = {}  # (comp, ra) -> rows
for tag, _ in COMPILERS:
    for ra in (0, 1):
        DATA[(tag, ra)] = parse(RA / f'{tag}_ra{ra}.txt')

md = []


def emit(s=''):
    md.append(s)


# ---------------------------------------------------------------- helpers
def compiler_geomean_table(ra: int, shape: str | None = None):
    algos = algos_for(shape) if shape else ALGOS
    for vt in TYPES:
        title = f'T = `{TYPE_LABEL[vt]}`'
        if shape:
            title += f', shape = `{shape}`'
        title += ':'
        emit(title)
        emit()
        emit('| Compiler | `nsg/seg` | `std/seg` | `std/nsg` |')
        emit('| --- | --- | --- | --- |')
        for tag, cname in COMPILERS:
            rows = DATA[(tag, ra)]
            cells = [f'{geomean([rows[(vt, a)][r] for a in algos if (vt, a) in rows]):.2f}'
                     for r in RATIOS]
            emit(f'| {cname} | ' + ' | '.join(cells) + ' |')
        emit()


def per_algo_cross_compiler_table(ra: int, vt: str, shape: str | None = None):
    algos = algos_for(shape) if shape else ALGOS
    title = f'T = `{TYPE_LABEL[vt]}`'
    if shape:
        title += f', shape = `{shape}`'
    title += ' (geomean of the three compilers):'
    emit(title)
    emit()
    emit('| Algorithm | `nsg/seg` | `std/seg` | `std/nsg` |')
    emit('| --- | --- | --- | --- |')
    per_ratio_all = {r: [] for r in RATIOS}
    for a in algos:
        cells = []
        for r in RATIOS:
            g = geomean([DATA[(tag, ra)][(vt, a)][r] for tag, _ in COMPILERS
                         if (vt, a) in DATA[(tag, ra)]])
            per_ratio_all[r].append(g)
            cells.append(f'{g:.2f}')
        label = algo_label(a) if shape else a
        emit(f'| `{label}` | ' + ' | '.join(cells) + ' |')
    emit('| **geomean** | ' + ' | '.join(f'**{geomean(per_ratio_all[r]):.2f}**' for r in RATIOS) + ' |')
    emit()


def annex_compiler_table(tag: str, vt: str):
    emit(f'T = `{TYPE_LABEL[vt]}`:')
    emit()
    emit('| Algorithm | `nsg/seg` B1 | `std/seg` B1 | `std/nsg` B1 | `nsg/seg` B2 | `std/seg` B2 | `std/nsg` B2 |')
    emit('| --- | --- | --- | --- | --- | --- | --- |')
    acc = {(ra, r): [] for ra in (0, 1) for r in RATIOS}
    for a in ALGOS:
        cells = []
        for ra in (0, 1):
            row = DATA[(tag, ra)].get((vt, a))
            for r in RATIOS:
                v = row[r] if row else float('nan')
                acc[(ra, r)].append(v)
                cells.append(f'{v:.2f}')
        emit(f'| `{a}` | ' + ' | '.join(cells) + ' |')
    emit('| **geomean** | ' + ' | '.join(
        f'**{geomean(acc[(ra, r)]):.2f}**' for ra in (0, 1) for r in RATIOS) + ' |')
    emit()


# ---------------------------------------------------------------- charts
def chart_per_algo(ra: int, vt: str, shape: str, fname: str, title: str):
    algos = algos_for(shape)
    labels = [algo_label(a) for a in algos]
    fig, ax = plt.subplots(figsize=(10, 4.8))
    x = range(len(algos))
    vals = [geomean([DATA[(tag, ra)][(vt, a)]['std_seg'] for tag, _ in COMPILERS
                     if (vt, a) in DATA[(tag, ra)]]) for a in algos]
    bars = ax.bar(list(x), vals, 0.68, color='#3b6ea5')
    ax.bar_label(bars, fmt='%.2f', fontsize=8, padding=2)
    ax.axhline(1.0, color='black', linewidth=0.8, linestyle='--')
    ax.set_xticks(list(x))
    ax.set_xticklabels(labels, rotation=45, ha='right', fontsize=9)
    ax.set_ylabel('std/seg  (higher = segmented faster)')
    ax.set_ylim(0, max(vals) * 1.14)
    ax.set_title(title)
    ax.spines['top'].set_visible(False)
    ax.spines['right'].set_visible(False)
    fig.tight_layout()
    fig.savefig(OUT / fname, dpi=130)
    plt.close(fig)


def chart_conclusions(vt: str, fname: str):
    labels = [cname for _, cname in COMPILERS]
    b1 = [geomean([DATA[(tag, 0)][(vt, a)]['std_seg'] for a in ALGOS
                   if (vt, a) in DATA[(tag, 0)]]) for tag, _ in COMPILERS]
    b2 = [geomean([DATA[(tag, 1)][(vt, a)]['std_seg'] for a in ALGOS
                   if (vt, a) in DATA[(tag, 1)]]) for tag, _ in COMPILERS]
    x = range(len(labels))

    fig, ax = plt.subplots(figsize=(7.0, 4.2))
    bars1 = ax.bar([xi - 0.19 for xi in x], b1, 0.38,
                   label='forward iterator', color='#a8c4de')
    bars2 = ax.bar([xi + 0.19 for xi in x], b2, 0.38,
                   label='random iterator', color='#2f6ba3')
    ax.bar_label(bars1, fmt='%.2f', fontsize=9, padding=2)
    ax.bar_label(bars2, fmt='%.2f', fontsize=9, padding=2)
    ax.axhline(1.0, color='black', linewidth=0.9, linestyle='--')
    ax.set_xticks(list(x))
    ax.set_xticklabels(labels)
    ax.set_ylabel('std/seg geomean')
    ax.set_ylim(0, max(max(b1), max(b2)) * 1.18)
    ax.set_title(f'Geomean std/seg per compiler — T = {TYPE_LABEL[vt]}')
    ax.legend(loc='upper left', frameon=True)
    ax.spines['top'].set_visible(False)
    ax.spines['right'].set_visible(False)
    fig.tight_layout()
    fig.savefig(OUT / fname, dpi=140)
    plt.close(fig)


def chart_annex(tag: str, cname: str, vt: str, fname: str):
    fig, ax = plt.subplots(figsize=(13, 5.0))
    x = range(len(ALGOS))
    b1 = [DATA[(tag, 0)].get((vt, a), {}).get('std_seg', float('nan')) for a in ALGOS]
    b2 = [DATA[(tag, 1)].get((vt, a), {}).get('std_seg', float('nan')) for a in ALGOS]
    ax.bar([xi - 0.19 for xi in x], b1, 0.38, label='forward iterator', color='#a8c4de')
    ax.bar([xi + 0.19 for xi in x], b2, 0.38, label='random iterator', color='#2f6ba3')
    ax.axhline(1.0, color='black', linewidth=0.8, linestyle='--')
    ax.set_title(f'{cname} — T = {TYPE_LABEL[vt]}', fontsize=11)
    ax.set_ylabel('std/seg')
    ax.set_xticks(list(x))
    ax.set_xticklabels(ALGOS, rotation=75, ha='right', fontsize=8)
    ax.legend(fontsize=8)
    ax.spines['top'].set_visible(False)
    ax.spines['right'].set_visible(False)
    fig.tight_layout()
    fig.savefig(OUT / fname, dpi=130)
    plt.close(fig)


def write_benchmark_section(ra: int, prefix: str) -> str:
    """Markdown body for the first/second benchmark tables + image links."""
    lines = []
    lines.append('Geomean per compiler, split by shape:')
    lines.append('')
    for shape in SHAPES:
        lines.append(f'#### Shape `{shape}` — {SHAPE_LABEL[shape]}')
        lines.append('')
        algos = algos_for(shape)
        for vt in TYPES:
            lines.append(f'T = `{TYPE_LABEL[vt]}`:')
            lines.append('')
            lines.append('| Compiler | `nsg/seg` | `std/seg` | `std/nsg` |')
            lines.append('| --- | --- | --- | --- |')
            for tag, cname in COMPILERS:
                rows = DATA[(tag, ra)]
                cells = [f'{geomean([rows[(vt, a)][r] for a in algos if (vt, a) in rows]):.2f}'
                         for r in RATIOS]
                lines.append(f'| {cname} | ' + ' | '.join(cells) + ' |')
            lines.append('')

        lines.append(
            f'Per-algorithm for `MyInt`, shape `{shape}` '
            '(geomean of the three compilers; per-compiler breakdowns are in the Annex):')
        lines.append('')
        lines.append('| Algorithm | `nsg/seg` | `std/seg` | `std/nsg` |')
        lines.append('| --- | --- | --- | --- |')
        per_ratio_all = {r: [] for r in RATIOS}
        for a in algos:
            cells = []
            for r in RATIOS:
                g = geomean([DATA[(tag, ra)][('MyInt', a)][r]
                             for tag, _ in COMPILERS
                             if ('MyInt', a) in DATA[(tag, ra)]])
                per_ratio_all[r].append(g)
                cells.append(f'{g:.2f}')
            lines.append(f'| `{algo_label(a)}` | ' + ' | '.join(cells) + ' |')
        lines.append('| **geomean** | ' + ' | '.join(
            f'**{geomean(per_ratio_all[r]):.2f}**' for r in RATIOS) + ' |')
        lines.append('')

        for vt in TYPES:
            sfx = TYPE_SUFFIX[vt]
            shsfx = SHAPE_SUFFIX[shape]
            fname = f'{prefix}_per_algo_{sfx}_{shsfx}.png'
            bench_name = 'First benchmark' if prefix == 'bench1' else 'Second benchmark'
            lines.append(
                f'![{bench_name}: std/seg per algorithm, '
                f'T = {TYPE_LABEL[vt]}, shape = {shape}]'
                f'({fname})')
            lines.append('')
    return '\n'.join(lines)


# ---------------------------------------------------------------- emit all
emit('# Generated tables')
emit()
emit('## Benchmark 1 (RA=0): compiler geomeans by shape')
emit()
for shape in SHAPES:
    compiler_geomean_table(0, shape)
emit('## Benchmark 1 (RA=0): per-algorithm by shape, cross-compiler geomean')
emit()
for shape in SHAPES:
    for vt in TYPES:
        per_algo_cross_compiler_table(0, vt, shape)
emit('## Benchmark 2 (RA=1): compiler geomeans by shape')
emit()
for shape in SHAPES:
    compiler_geomean_table(1, shape)
emit('## Benchmark 2 (RA=1): per-algorithm by shape, cross-compiler geomean')
emit()
for shape in SHAPES:
    for vt in TYPES:
        per_algo_cross_compiler_table(1, vt, shape)
emit('## Annex tables (B1 = first benchmark, B2 = second benchmark)')
emit()
for tag, cname in COMPILERS:
    emit(f'### {cname}')
    emit()
    for vt in TYPES:
        annex_compiler_table(tag, vt)

(OUT / 'article_tables.md').write_text('\n'.join(md), encoding='utf-8')
(OUT / 'bench1_section.md').write_text(write_benchmark_section(0, 'bench1'), encoding='utf-8')
(OUT / 'bench2_section.md').write_text(write_benchmark_section(1, 'bench2'), encoding='utf-8')

for vt in TYPES:
    sfx = TYPE_SUFFIX[vt]
    for shape in SHAPES:
        shsfx = SHAPE_SUFFIX[shape]
        chart_per_algo(
            0, vt, shape,
            f'bench1_per_algo_{sfx}_{shsfx}.png',
            f'First benchmark: std/seg per algorithm, T = {TYPE_LABEL[vt]}, '
            f'shape = {shape} (geomean of GCC 16, Clang 22, MSVC 2026)')
        chart_per_algo(
            1, vt, shape,
            f'bench2_per_algo_{sfx}_{shsfx}.png',
            f'Second benchmark: std/seg per algorithm, T = {TYPE_LABEL[vt]}, '
            f'shape = {shape} (geomean of GCC 16, Clang 22, MSVC 2026)')
    chart_conclusions(vt, f'conclusions_std_seg_{sfx}.png')
    for tag, cname in COMPILERS:
        chart_annex(tag, cname, vt, f'annex_{tag}_{sfx}.png')

# Remove superseded combined-type charts if present
for vt in TYPES:
    sfx = TYPE_SUFFIX[vt]
    for name in (f'bench1_per_algo_{sfx}.png', f'bench2_per_algo_{sfx}.png'):
        p = OUT / name
        if p.exists():
            p.unlink()

print('written to', OUT)
