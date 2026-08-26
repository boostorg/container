#!/usr/bin/env python3
"""Median-of-reps A/B report from the logs 407_bench.sh leaves in /tmp/pf39bench.

Usage: 408_report.py <group> <label> [label...]
Prints, per container section and per timing column, the pre/post medians in ns
and the post-vs-pre change.  A positive change means the fix made it slower.
"""
import glob, os, re, statistics, sys

OUT = '/tmp/pf39bench'
group = sys.argv[1]
labels = sys.argv[2:]

# label -> (container, column) -> list of per-rep values
data = {}

row_re = re.compile(r'^(.{28})\s*([-\d.]+)\s+([-\d.]+)\s+([-\d.]+)\s+([-\d.]+)\s+([-\d.]+)\s+([-\d.]+)\s*$')

for path in sorted(glob.glob(os.path.join(OUT, '*.g%s.r*.log' % group))):
    base = os.path.basename(path)
    m = re.match(r'^(.*)\.(pre|post)\.g%s\.r(\d+)\.log$' % group, base)
    if not m:
        continue
    cc, side, rep = m.group(1), m.group(2), m.group(3)
    cont = '?'
    for line in open(path):
        line = line.rstrip('\n')
        cm = re.match(r'^--- (.*) ---$', line)
        if cm:
            cont = cm.group(1)
            continue
        rm = row_re.match(line)
        if not rm:
            continue
        label = rm.group(1).strip()
        if label not in labels:
            continue
        seg, std, nsg = float(rm.group(5)), float(rm.group(6)), float(rm.group(7))
        for col, val in (('seg', seg), ('nsg', nsg), ('std', std)):
            data.setdefault((cc, cont, label, col), {}).setdefault(side, []).append(val)

print('group %s   (ns/element, median of reps; +%% = post slower)' % group)
hdr = '%-10s %-22s %-18s %-4s %9s %9s %8s  %s'
print(hdr % ('compiler', 'container', 'row', 'col', 'pre', 'post', 'change', 'reps'))
for key in sorted(data):
    cc, cont, label, col = key
    if col == 'std':
        continue
    sides = data[key]
    if 'pre' not in sides or 'post' not in sides:
        continue
    pre = statistics.median(sides['pre'])
    post = statistics.median(sides['post'])
    ch = (post - pre) / pre * 100.0 if pre else 0.0
    flag = '  <== >10%' if abs(ch) > 10.0 else ''
    print((hdr % (cc, cont, label, col, '%.4f' % pre, '%.4f' % post,
                  '%+.1f%%' % ch, '%d/%d' % (len(sides['pre']), len(sides['post'])))) + flag)
