#!/bin/bash
set -u
BR=/mnt/d/Data/LocalGit/boost
EX=$BR/libs/container/experimental
S=$EX/cursor_build/g32/applyfix
SH=$S/pre
RAW=$S/raw3
REPS=5

echo "=== shadow ('before') vs tree ('after'), EOL-insensitive diff ==="
for h in segmented_copy_if segmented_remove_copy segmented_remove_copy_if segmented_partition_copy; do
   echo "--- $h.hpp"
   diff -u --strip-trailing-cr $SH/boost/container/experimental/$h.hpp \
        $BR/boost/container/experimental/$h.hpp | tail -n +3 | sed 's/^/   /'
done

for G in 25 15 30; do
   for CXX in g++-16 clang++-22; do
      echo
      echo "================ group $G  $CXX : median of $REPS pinned runs ================"
      python3 - "$RAW" "$G" "$CXX" "$REPS" <<'PY'
import sys, re, statistics
RAW, G, CXX, REPS = sys.argv[1], sys.argv[2], sys.argv[3], int(sys.argv[4])
row = re.compile(r'^(\S.*?)\s{2,}([\d.]+)\s+([\d.]+)\s+([\d.]+)\s+([\d.]+)\s+([\d.]+)\s+([\d.]+)\s*$')
def load(v):
    runs, order = [], []
    for r in range(1, REPS+1):
        d = {}
        try:
            for line in open(f"{RAW}/r_{G}_{CXX}_{v}_{r}.txt"):
                m = row.match(line)
                if m:
                    k = m.group(1).strip()
                    if k.startswith('<') or 'geomean' in k: continue
                    d[k] = (float(m.group(5)), float(m.group(7)))
                    if r == 1: order.append(k)
        except FileNotFoundError:
            pass
        runs.append(d)
    out = {}
    for k in order:
        segs = [rr[k][0] for rr in runs if k in rr]
        nsgs = [rr[k][1] for rr in runs if k in rr]
        if segs:
            out[k] = (statistics.median(segs), statistics.median(nsgs),
                      (max(segs)-min(segs))/statistics.median(segs)*100.0 if statistics.median(segs) else 0.0)
    return out, order
pre, order = load("pre")
post, _    = load("post")
if not order:
    print("   (no rows parsed)")
else:
    print(f"{'algo':<30}{'before':>9}{'after':>9}{'delta':>8}{'spread':>8}   {'nsg/seg bef':>11}{'nsg/seg aft':>12}")
    for k in order:
        if k not in pre or k not in post: continue
        sp, npre, spread_p = pre[k]
        sq, npost, spread_q = post[k]
        d = (sq-sp)/sp*100.0 if sp else 0.0
        rp = npre/sp if sp else 0.0
        rq = npost/sq if sq else 0.0
        sprd = max(spread_p, spread_q)
        flag = "  <<<" if abs(d) > 3.0 else ""
        print(f"{k:<30}{sp:9.4f}{sq:9.4f}{d:+7.1f}%{sprd:7.1f}%   {rp:11.2f}{rq:12.2f}{flag}")
PY
   done
done
