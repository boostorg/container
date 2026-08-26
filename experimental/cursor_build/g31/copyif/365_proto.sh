#!/bin/bash
# 365: build + measure the shadow-header prototypes.
#  - correctness: segmented_copy_if_test.cpp, both compilers
#  - conformance: predicate-application counts (362 probe rebuilt per variant)
#  - performance: benchmark group 25, pinned median of 5
set -u
BR=/mnt/d/Data/LocalGit/boost
EX=$BR/libs/container/experimental
WD=$EX/cursor_build/g31/copyif

python3 "$WD/365_mkproto.py" || exit 1
echo

BFLAGS="-std=c++20 -O3 -DNDEBUG -DBENCH_ON -DBOOST_CONTAINER_BENCH_SEGMENTED_GROUP=25 -falign-functions=64 -falign-loops=64"

cd "$EX" || exit 1

for V in p0 p1 p2 p3; do
  for CXX in g++-16 clang++-22; do
    echo "===== $V / $CXX ====="
    # correctness gate
    $CXX -std=c++20 -O2 -DNDEBUG -I"$WD/$V" -I../../.. segmented_copy_if_test.cpp \
        -o "$WD/365_test_${V}_${CXX}.elf" 2>&1 | head -15
    if [ -x "$WD/365_test_${V}_${CXX}.elf" ]; then
      if "$WD/365_test_${V}_${CXX}.elf" > "$WD/365_test_${V}_${CXX}.log" 2>&1; then
        echo "  segmented_copy_if_test: PASS"
      else
        echo "  segmented_copy_if_test: FAIL (rc=$?)"; tail -5 "$WD/365_test_${V}_${CXX}.log"
      fi
    else
      echo "  segmented_copy_if_test: BUILD FAILED"
    fi
    # predicate counts
    $CXX -std=c++20 -O2 -DNDEBUG -I"$WD/$V" -I"$BR" "$WD/362_predcount.cpp" \
        -o "$WD/365_cnt_${V}_${CXX}.elf" 2>&1 | head -10
    if [ -x "$WD/365_cnt_${V}_${CXX}.elf" ]; then
      "$WD/365_cnt_${V}_${CXX}.elf" > "$WD/365_cnt_${V}_${CXX}.txt"
      echo "  pred counts (71-vs-64 case + block128 hit/miss + depth2):"
      grep -E 'deque dst .*block=8 .*n=64|block=128 .*every=1|block=128 .*every=0|seg2_vector' \
         "$WD/365_cnt_${V}_${CXX}.txt" | sed 's/^/    /'
    fi
    # benchmark
    $CXX $BFLAGS -I"$WD/$V" -I../../.. bench_segmented_algos.cpp -o "$WD/365_bench_${V}_${CXX}.elf" 2>&1 | head -10
    if [ -x "$WD/365_bench_${V}_${CXX}.elf" ]; then
      for r in 1 2 3 4 5; do
        nice -n -5 taskset -c 3 "$WD/365_bench_${V}_${CXX}.elf" > "$WD/365_bench_${V}_${CXX}_r${r}.txt" 2>&1
      done
    fi
    echo
  done
done

python3 - "$WD" <<'PY'
import sys, os, statistics
wd = sys.argv[1]
rows = ["copy(1S)","copy(2S)","copy(1+2S)",
        "copy_if(1S hit)","copy_if(2S hit)","copy_if(1+2S hit)",
        "copy_if(1S miss)","copy_if(2S miss)","copy_if(1+2S miss)"]
for cxx in ("g++-16","clang++-22"):
    print("\n================= %s : seg ns, median of 5 pinned runs =================" % cxx)
    print("%-22s %9s %9s %9s %9s %10s" % ("algo","p0","p1","p2","p3","nsg(p0)"))
    med = {}
    for v in ("p0","p1","p2","p3"):
        d = {}
        for r in range(1,6):
            p = os.path.join(wd, "365_bench_%s_%s_r%d.txt" % (v, cxx, r))
            if not os.path.exists(p): continue
            for line in open(p):
                for name in rows:
                    if line.startswith(name+" "):
                        f = line.split()[-6:]
                        d.setdefault(name, []).append((float(f[3]), float(f[5])))
                        break
        med[v] = {k: (statistics.median([x[0] for x in val]),
                      statistics.median([x[1] for x in val])) for k, val in d.items()}
    for name in rows:
        vals = []
        for v in ("p0","p1","p2","p3"):
            vals.append(med[v].get(name, (float('nan'),))[0])
        nsg = med["p0"].get(name, (0,float('nan')))[1]
        print("%-22s %9.3f %9.3f %9.3f %9.3f %10.3f" % (name, vals[0], vals[1], vals[2], vals[3], nsg))
    print("%-22s %9s %9s %9s %9s" % ("nsg/seg 2S miss","","","",""))
    for name in ("copy_if(2S hit)","copy_if(2S miss)"):
        nsg = med["p0"].get(name,(1,1))[1]
        print("  nsg/seg %-14s %9.2f %9.2f %9.2f %9.2f" % (name,
            nsg/med["p0"][name][0], nsg/med["p1"][name][0],
            nsg/med["p2"][name][0], nsg/med["p3"][name][0]))
PY
