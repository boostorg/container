#!/bin/bash
# Symbol size of the specialisations that contain the affected leaves, plus
# total .text, for base vs the unrolled overlay.  The leaves themselves are
# FORCEINLINE, so they are measured through their enclosing measure_batch
# specialisation in the group-25 benchmark binary.
set -u
BIN=/mnt/d/Data/LocalGit/boost/libs/container/experimental/cursor_build/g37/unroll/bin
GROUP=${GROUP:-25}
BASE=${BASE:-base}
VAR=${VAR:-p2_all}

dump() { # $1 = binary
   nm -C -S --defined-only "$1" 2>/dev/null | awk '{ sz = strtonum("0x" $2); name=""; for(i=4;i<=NF;i++) name = name (i>4?" ":"") $i; print sz "\t" name }'
}

for cc in g++-16 clang++-22; do
  b=$BIN/b_${cc}_${BASE}_g${GROUP}
  a=$BIN/b_${cc}_${VAR}_g${GROUP}
  [ -x "$b" ] && [ -x "$a" ] || { echo "missing binaries for $cc"; continue; }
  echo "================= $cc  ($BASE -> $VAR) ================="
  export CCN=$cc
  dump "$b" | sort -k2 > /tmp/sb_$cc.txt
  dump "$a" | sort -k2 > /tmp/sa_$cc.txt
  echo "--- text segment ---"
  echo "base : $(size -A $b | awk '$1==".text"{print $2}') bytes .text, file $(stat -c %s $b)"
  echo "after: $(size -A $a | awk '$1==".text"{print $2}') bytes .text, file $(stat -c %s $a)"
  for pat in seg_copy_if seg_remove_copy_if 'seg_remove_copy<' seg_partition_copy; do
     echo "--- symbols matching: $pat ---"
     python3 - "$pat" <<'EOF'
import sys, re, os
pat = sys.argv[1]
def load(p):
    d = {}
    for line in open(p):
        sz, name = line.rstrip('\n').split('\t', 1)
        d[name] = d.get(name, 0) + int(sz)
    return d
cc = os.environ['CCN']
b = load('/tmp/sb_%s.txt' % cc); a = load('/tmp/sa_%s.txt' % cc)
tb = ta = 0
rows = []
for name in sorted(set(b) | set(a)):
    if pat not in name:
        continue
    x, y = b.get(name, 0), a.get(name, 0)
    tb += x; ta += y
    short = name
    if len(short) > 110: short = short[:107] + '...'
    rows.append((x, y, short))
for x, y, s in sorted(rows, key=lambda r: -max(r[0], r[1]))[:8]:
    print("  %7d -> %7d  %+6d  %s" % (x, y, y - x, s))
if tb or ta:
    print("  TOTAL %d -> %d (%+d, %+.1f%%) over %d symbols"
          % (tb, ta, ta - tb, 100.0 * (ta - tb) / tb if tb else 0, len(rows)))
else:
    print("  (no matching symbols)")
EOF
  done
done
