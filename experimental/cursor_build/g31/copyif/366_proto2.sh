#!/bin/bash
# 366: build + measure prototypes p4 / p5 (restructured random-access leaf),
# then summarise every variant whose benchmark runs are on disk.
set -u
BR=/mnt/d/Data/LocalGit/boost
EX=$BR/libs/container/experimental
WD=$EX/cursor_build/g31/copyif

python3 "$WD/365_mkproto.py" p4 p5 || exit 1
echo

BFLAGS="-std=c++20 -O3 -DNDEBUG -DBENCH_ON -DBOOST_CONTAINER_BENCH_SEGMENTED_GROUP=25 -falign-functions=64 -falign-loops=64"
cd "$EX" || exit 1

for V in p4 p5; do
  for CXX in g++-16 clang++-22; do
    echo "===== $V / $CXX ====="
    $CXX -std=c++20 -O2 -DNDEBUG -I"$WD/$V" -I../../.. segmented_copy_if_test.cpp \
        -o "$WD/365_test_${V}_${CXX}.elf" 2>&1 | head -15
    if [ -x "$WD/365_test_${V}_${CXX}.elf" ]; then
      if "$WD/365_test_${V}_${CXX}.elf" > "$WD/365_test_${V}_${CXX}.log" 2>&1; then
        echo "  segmented_copy_if_test: PASS"
      else
        echo "  segmented_copy_if_test: FAIL"; tail -5 "$WD/365_test_${V}_${CXX}.log"
      fi
    else
      echo "  segmented_copy_if_test: BUILD FAILED"
    fi
    $CXX -std=c++20 -O2 -DNDEBUG -I"$WD/$V" -I"$BR" "$WD/362_predcount.cpp" \
        -o "$WD/365_cnt_${V}_${CXX}.elf" 2>&1 | head -10
    if [ -x "$WD/365_cnt_${V}_${CXX}.elf" ]; then
      "$WD/365_cnt_${V}_${CXX}.elf" > "$WD/365_cnt_${V}_${CXX}.txt"
      echo "  pred counts:"
      grep -E 'deque dst .*block=8 .*n=64|block=128 .*every=1|seg2_vector|seg_vector' \
         "$WD/365_cnt_${V}_${CXX}.txt" | sed 's/^/    /'
    fi
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
        "copy_if(1S miss)","copy_if(2S miss)","copy_if(1+2S miss)",
        "copy_n(2S)","remove_copy(2S hit)","transform(2S)"]
VS = ["p0","p1","p2","p3","p4","p5"]
for cxx in ("g++-16","clang++-22"):
    med = {}
    for v in VS:
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
        med[v] = {k:(statistics.median([x[0] for x in val]),
                     statistics.median([x[1] for x in val])) for k,val in d.items()}
    have = [v for v in VS if med[v]]
    print("\n============ %s : seg ns/element, median of 5 pinned runs ============" % cxx)
    print("%-22s %s %9s" % ("algo", " ".join("%8s"%v for v in have), "nsg(p0)"))
    for name in rows:
        cells = " ".join("%8.3f" % med[v][name][0] if name in med[v] else "%8s"%"-" for v in have)
        nsg = med["p0"].get(name,(0,float('nan')))[1]
        print("%-22s %s %9.3f" % (name, cells, nsg))
    print("\n%-22s %s" % ("nsg/seg (>1 = seg wins)", " ".join("%8s"%v for v in have)))
    for name in ("copy_if(2S hit)","copy_if(2S miss)","copy_if(1+2S hit)","copy_if(1S hit)"):
        nsg = med["p0"].get(name,(1,1))[1]
        cells = " ".join("%8.2f" % (nsg/med[v][name][0]) if name in med[v] else "%8s"%"-" for v in have)
        print("%-22s %s" % (name, cells))
PY
