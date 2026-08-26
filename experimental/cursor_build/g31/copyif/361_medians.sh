#!/bin/bash
# 361: 5 pinned repeats of the group-25 benchmark per compiler; median of the
# copy/copy_if rows (seg ns, std ns, nsg ns) recomputed from the medians.
set -u
BR=/mnt/d/Data/LocalGit/boost
EX=$BR/libs/container/experimental
WD=$EX/cursor_build/g31/copyif
cd "$EX" || exit 1

for CXX in g++-16 clang++-22; do
  for r in 1 2 3 4 5; do
    nice -n -5 taskset -c 3 "$WD/a_${CXX}.elf" > "$WD/361_${CXX}_r${r}.txt" 2>&1
  done
done

python3 - "$WD" <<'PY'
import sys, os, statistics
wd = sys.argv[1]
rows_of_interest = [
 "copy(1S)","copy(2S)","copy(1+2S)",
 "copy_if(1S hit)","copy_if(2S hit)","copy_if(1+2S hit)",
 "copy_if(1S miss)","copy_if(2S miss)","copy_if(1+2S miss)",
]
for cxx in ("g++-16","clang++-22"):
    data = {}
    for r in range(1,6):
        p = os.path.join(wd, "361_%s_r%d.txt" % (cxx, r))
        for line in open(p):
            for name in rows_of_interest:
                if line.startswith(name+" ") or line.startswith(name+"\t"):
                    f = line.split()
                    # label may contain spaces: last 6 fields are the numbers
                    nums = f[-6:]
                    seg, std, nsg = float(nums[3]), float(nums[4]), float(nums[5])
                    data.setdefault(name, []).append((seg,std,nsg))
                    break
    print("\n===== %s : median of 5 pinned runs (taskset -c 3) =====" % cxx)
    print("%-22s %10s %10s %10s %10s %10s %10s" %
          ("algo","seg ns","std ns","nsg ns","nsg/seg","std/seg","std/nsg"))
    for name in rows_of_interest:
        v = data.get(name)
        if not v:
            print("%-22s  MISSING" % name); continue
        seg = statistics.median([x[0] for x in v])
        std = statistics.median([x[1] for x in v])
        nsg = statistics.median([x[2] for x in v])
        print("%-22s %10.3f %10.3f %10.3f %10.2f %10.2f %10.2f" %
              (name, seg, std, nsg, nsg/seg, std/seg, std/nsg))
    # spread check
    print("--- per-run seg ns spread ---")
    for name in rows_of_interest:
        v = data.get(name)
        if not v: continue
        s = [x[0] for x in v]
        print("%-22s %s  (min %.3f max %.3f spread %.1f%%)" %
              (name, " ".join("%.3f"%x for x in s), min(s), max(s),
               100.0*(max(s)-min(s))/min(s)))
PY
