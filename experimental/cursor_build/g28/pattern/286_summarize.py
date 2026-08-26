# Median 'seg ns' per (compiler, variant, group, value type, algo) across
# bench launches, printed as base vs variant deltas.
# Usage: 286_summarize.py <runs-dir>
import os, re, sys, statistics

runs = sys.argv[1]
row_re = re.compile(r'^(\S+)\s+(\d+\.\d+)\s+(\d+\.\d+)\s+(\d+\.\d+)\s+(\d+\.\d+)\s+(\d+\.\d+)\s+(\d+\.\d+)\s*$')
vt_re = re.compile(r'^===== Group (\d+):.*\[(\S+)\] =====')
geo_re = re.compile(r'^group geomean\s')

# data[(cc,grp)][variant][(vt,algo)] = [seg_ns...]
data = {}
launches = {}
for fn in sorted(os.listdir(runs)):
    m = re.match(r'bench\.(.+?)\.(base|T|F|W|H)\.g(\d+)\.run(\d+)\.txt$', fn)
    if not m:
        continue
    cc, var, grp, run = m.group(1), m.group(2), m.group(3), m.group(4)
    key = (cc, grp)
    data.setdefault(key, {}).setdefault(var, {})
    launches.setdefault((cc, var, grp), set()).add(run)
    vt = '?'
    for line in open(os.path.join(runs, fn)):
        mm = vt_re.match(line)
        if mm:
            vt = mm.group(2)
            continue
        mm = row_re.match(line)
        if mm and not geo_re.match(line):
            algo = mm.group(1)
            seg = float(mm.group(5))
            data[key][var].setdefault((vt, algo), []).append(seg)

for (cc, grp) in sorted(data):
    vars_ = data[(cc, grp)]
    if 'base' not in vars_:
        continue
    others = [v for v in ('T', 'F', 'H', 'W') if v in vars_]
    print("\n===== %s  group %s  (launches: %s) =====" %
          (cc, grp, {v: len(launches[(cc, v, grp)]) for v in ['base'] + others}))
    hdr = "%-28s %-10s %10s" % ("vt", "algo", "base")
    for v in others:
        hdr += "%10s %8s" % (v, "d%")
    keys = sorted(vars_['base'])
    print("%-22s %-26s %10s" % ("vt", "algo", "base") +
          "".join("%10s %8s" % (v, v + " d%") for v in others))
    for k in keys:
        base_med = statistics.median(vars_['base'][k])
        line = "%-22s %-26s %10.3f" % (k[0], k[1], base_med)
        for v in others:
            if k in vars_[v]:
                med = statistics.median(vars_[v][k])
                d = (med / base_med - 1.0) * 100.0 if base_med else 0.0
                line += "%10.3f %+7.1f%%" % (med, d)
            else:
                line += "%10s %8s" % ("-", "-")
        print(line)
