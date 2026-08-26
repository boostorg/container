#!/bin/bash
# Static codegen comparison of segment-walking variants.
# base = real tree; E/T/F/W = g28 shadow overlays picked up via -I ordering.
set -u
BR=/mnt/d/Data/LocalGit/boost
G=$BR/libs/container/experimental/cursor_build/g28/pattern
EX=$BR/libs/container/experimental
O=/tmp/g28static
OUT=$G/out
mkdir -p $O $OUT
PYC=/tmp/281c.py
tr -d '\r' < $G/281_count.py > $PYC

OPTLIST=${OPTLIST:--O2}
VARIANTS=${VARIANTS:-base E T F W}

for OPT in $OPTLIST; do
for CC in g++-16 clang++-22; do
   for V in $VARIANTS; do
      if [ $V = base ]; then INC="-I$BR"; EXTRA=""; else INC="-I$G/shadow_$V -I$BR"; EXTRA="-DBOOST_CONTAINER_G28_EXPECT_SHADOW"; fi
      obj=$O/probe.$CC.$V$OPT.o
      if ! $CC -std=c++20 $OPT -DNDEBUG $EXTRA $INC -I$EX \
           -c $G/280_probe.cpp -o $obj 2>$O/e.log; then
         echo "==== $CC $V $OPT : COMPILE FAILED"; grep -m5 'error' $O/e.log | sed 's/^/    /'
         continue
      fi
      objdump -d --no-show-raw-insn $obj > $OUT/lst.$CC.$V$OPT.txt
      echo
      echo "==================== $CC  variant=$V  $OPT ===================="
      size -A $obj | awk '$1 ~ /^\.text/{s+=$2} END{printf "  .text* bytes = %d\n", s}'
      # per-symbol byte sizes of walker/probe symbols
      nm -S --size-sort -C $obj 2>/dev/null | grep -E 'probe_|detail_algo' | \
         awk '{printf "  bytes=%-6d %s\n", strtonum("0x"$2), substr($0, index($0,$4))}' | head -40
      python3 $PYC $OUT/lst.$CC.$V$OPT.txt probe_ detail_algo
   done
done
done
