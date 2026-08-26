#!/bin/bash
# Same checks as 215 but against the REAL (now edited) header, plus a diff
# confirming the applied edit is exactly the measured shadow.
set -u
BR=/mnt/d/Data/LocalGit/boost
G=/tmp/g23
SRC=$BR/libs/container/experimental/cursor_build/g23/flag
EX=$BR/libs/container/experimental
O=/tmp/g23pcf
SH=$O/shadow
rm -rf $O; mkdir -p $O

python3 $G/211_mkshadow_pc.py $SH/boost/container/experimental/segmented_partition_copy.hpp >/dev/null 2>&1
if [ -f $SH/boost/container/experimental/segmented_partition_copy.hpp ]; then
   echo "NOTE: shadow still buildable from the real header -> edit NOT applied yet"
   exit 1
fi
python3 - <<'EOF'
real = open("/mnt/d/Data/LocalGit/boost/boost/container/experimental/segmented_partition_copy.hpp", newline='').read().replace("\r\n","\n")
print("real header: 'first == last || r.fourth' occurrences =", real.count("first == last || r.fourth"))
print("real header: '!r.fourth' occurrences               =", real.count("!r.fourth"))
print("real header: 'false_output_full' occurrences       =", real.count("false_output_full"))
print("real header: 'true_output_full' occurrences        =", real.count("true_output_full"))
EOF

fails=0

echo
echo "--- leaf contract probe (real header) ---"
for CC in g++-16 clang++-22; do
   $CC -std=c++20 -O2 -Wall -Wextra -I$BR -I$EX \
       $SRC/214_pc_contract.cpp -o $O/contract.$CC 2>$O/contract.$CC.err
   if [ $? -ne 0 ]; then echo "  $CC: BUILD FAILED"; head -20 $O/contract.$CC.err; fails=1; continue; fi
   [ -s $O/contract.$CC.err ] && { echo "  $CC: WARNINGS"; cat $O/contract.$CC.err; fails=1; }
   printf "  %-12s " $CC
   $O/contract.$CC || { echo "  RUN FAILED"; fails=1; }
done

echo
echo "--- segmented_partition_copy_test.cpp + segmented_partition_test.cpp ---"
for T in segmented_partition_copy_test segmented_partition_test; do
   for CC in g++-16 clang++-22; do
      for STD in c++03 c++11 c++17 c++20; do
         log=$O/$T.$CC.$STD.log
         $CC -std=$STD -O2 -DNDEBUG -Wall -Wextra -I$BR -I$EX \
             $EX/$T.cpp -o $O/$T.$CC.$STD >$log 2>&1
         if [ $? -ne 0 ]; then printf "  %-30s %-12s %-6s BUILD FAILED\n" $T $CC $STD; head -15 $log; fails=1; continue; fi
         if [ -s $log ]; then printf "  %-30s %-12s %-6s WARNINGS\n" $T $CC $STD; head -15 $log; fails=1; continue; fi
         if $O/$T.$CC.$STD >$log.run 2>&1; then printf "  %-30s %-12s %-6s pass\n" $T $CC $STD;
         else printf "  %-30s %-12s %-6s RUN FAILED\n" $T $CC $STD; tail -15 $log.run; fails=1; fi
      done
   done
done

echo
echo "--- sanitizers (asan+ubsan, c++20) ---"
for CC in g++-16 clang++-22; do
   for T in $EX/segmented_partition_copy_test.cpp $EX/segmented_partition_test.cpp $SRC/214_pc_contract.cpp; do
      b=$(basename $T .cpp)
      $CC -std=c++20 -O1 -g -Wall -Wextra -fsanitize=address,undefined \
          -fno-omit-frame-pointer -I$BR -I$EX $T -o $O/s.$CC.$b >$O/s.$CC.$b.log 2>&1
      if [ $? -ne 0 ]; then printf "  %-12s %-34s BUILD FAILED\n" $CC $b; head -15 $O/s.$CC.$b.log; fails=1; continue; fi
      if $O/s.$CC.$b >$O/s.$CC.$b.run 2>&1; then printf "  %-12s %-34s clean\n" $CC $b;
      else printf "  %-12s %-34s SANITIZER FAILURE\n" $CC $b; tail -20 $O/s.$CC.$b.run; fails=1; fi
   done
done

echo
echo "--- branch structure of partition_copy_false_dispatch [seg], clang depth 2 ---"
clang++-22 -std=c++20 -O2 -DNDEBUG -DDEPTH=2 -I$BR -I$EX -c $SRC/210_pc_probe.cpp -o $O/d2.o
objdump -d --no-show-raw-insn $O/d2.o > $O/d2.txt
python3 - "$O/d2.txt" <<'EOF'
import re, sys, subprocess
txt = open(sys.argv[1]).read().splitlines()
cur=None; body=[]
for l in txt:
   m = re.match(r'^[0-9a-f]+ <(.+)>:$', l)
   if m:
      if cur and 'false_dispatch' in subprocess.run(['c++filt'],input=cur,capture_output=True,text=True).stdout:
         break
      cur=m.group(1); body=[]; continue
   if cur: body.append(l)
for l in body:
   if re.search(r'\b(cmp|test|j[a-z]+|call)', l):
      print("   " + l.strip())
EOF

echo
[ $fails -eq 0 ] && echo "216: ALL OK" || echo "216: FAILURES PRESENT"
exit $fails
