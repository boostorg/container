#!/bin/bash
# Benchmark confirmation: groups 10/15/20/25/30, before (bin_before) vs after,
# both compilers, 5 pinned launches each, medians of the `seg ns` column.
set -u
BR=/mnt/d/Data/LocalGit/boost
EX=$BR/libs/container/experimental
G=$BR/libs/container/experimental/cursor_build/g29/rollout
BA=$G/bin_after
export RUNS=$G/runs
mkdir -p $BA $RUNS
ALIGN_CLANG="-falign-functions=64 -falign-loops=64"

echo "=== build after-bench binaries ==="
for CC in g++-16 clang++-22; do
   EXTRA=""
   [ "$CC" = "clang++-22" ] && EXTRA="$ALIGN_CLANG"
   for GRP in 10 15 20 25 30; do
      $CC -std=c++20 -O2 -DNDEBUG -DBENCH_ON -DBOOST_CONTAINER_BENCH_SEGMENTED_GROUP=$GRP \
          $EXTRA -I$BR -I$EX $EX/bench_segmented_algos.cpp -o $BA/bench.$GRP.$CC 2>$BA/bench.$GRP.$CC.log \
         || { echo "BENCH BUILD FAIL $GRP $CC"; exit 1; }
   done
   echo "  $CC done"
done

if sudo -n chrt -f 90 taskset -c 3 true 2>/dev/null; then
   PIN="sudo -n chrt -f 90 taskset -c 3"
else
   PIN="nice -n -5 taskset -c 3"
fi
echo "pin: $PIN"

echo "=== run: 5 launches each ==="
for GRP in 10 15 20 25 30; do
   for CC in g++-16 clang++-22; do
      for i in 1 2 3 4 5; do
         $PIN $G/bin_before/bench.$GRP.$CC > $RUNS/before.$GRP.$CC.$i.txt 2>&1
         $PIN $BA/bench.$GRP.$CC           > $RUNS/after.$GRP.$CC.$i.txt  2>&1
      done
      echo "  group $GRP $CC done"
   done
done

echo "=== medians of seg ns (delta = after vs before) ==="
python3 - <<'EOF'
import statistics, os
runs = os.environ['RUNS']
def rows(path):
    out=[]
    for ln in open(path, errors='replace'):
        f=ln.split()
        if len(f)>=7:
            tail=f[-6:]
            try: nums=[float(x) for x in tail]
            except ValueError: continue
            name=' '.join(f[:-6])
            out.append((name, nums[3]))
    return out
for grp in [10,15,20,25,30]:
    for cc in ['g++-16','clang++-22']:
        data={}
        for phase in ['before','after']:
            per={}
            for i in range(1,6):
                p=f'{runs}/{phase}.{grp}.{cc}.{i}.txt'
                for idx,(name,v) in enumerate(rows(p)):
                    per.setdefault((idx,name),[]).append(v)
            data[phase]={k:statistics.median(v) for k,v in per.items()}
        print(f'--- group {grp} {cc} ---')
        for k in sorted(set(data['before'])|set(data['after'])):
            b=data['before'].get(k); a=data['after'].get(k)
            if b is None or a is None or b==0: continue
            d=(a-b)/b*100
            flag=' <<<' if d>5 else ''
            print(f'  {k[1]:<30} before={b:9.3f} after={a:9.3f} delta={d:+6.1f}%{flag}')
EOF
echo "306 done"
