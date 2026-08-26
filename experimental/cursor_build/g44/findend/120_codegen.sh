#!/bin/bash
# Builds 110_codegen.cpp against every variant include root and reports, per
# symbol: nm size, real instruction count (alignment padding excluded), code
# bytes excluding padding, and whether a cmov survived.
set -u

ROOT=/mnt/d/Data/LocalGit/boost
HERE=$ROOT/libs/container/experimental/cursor_build/g44/findend
EXP=$ROOT/libs/container/experimental
OUTDIR=$HERE/out/codegen
mkdir -p "$OUTDIR"

python3 "$HERE/100_mkvariants.py" || exit 1

SYMS="verify_leaf verify_walk1 verify_walk2 fe_bwd_seg fe_fwd_seg fe_bwd_seg2 fe_deque"
VARIANTS="base a b c d e"

printf '%-10s %-4s %-4s %-14s %7s %7s %7s %5s\n' \
   compiler opt var symbol nmsize insns netby cmov

for CXX in g++-16 clang++-22; do
  for OPT in -O2 -O3; do
    for V in $VARIANTS; do
      FLAG=""
      [ "$V" != "base" ] && FLAG="-DFE_HAS_FLAG"
      OBJ=$OUTDIR/${CXX}_${OPT#-}_$V.o
      LOG=$OUTDIR/${CXX}_${OPT#-}_$V.log
      if ! $CXX -std=c++17 $OPT -DNDEBUG $FLAG -c \
        -I"$HERE/inc_$V" -I"$ROOT" -I"$EXP" \
        "$HERE/110_codegen.cpp" -o "$OBJ" 2>"$LOG"; then
        printf '%-10s %-4s %-4s %-14s BUILD FAILED\n' "$CXX" "$OPT" "$V" ""
        head -20 "$LOG"
        continue
      fi
      DIS=$OUTDIR/${CXX}_${OPT#-}_$V.dis
      objdump -d --no-show-raw-insn -C "$OBJ" > "$DIS"
      for S in $SYMS; do
        NM=$(nm --size-sort -S "$OBJ" 2>/dev/null | awk -v s="$S" '$4==s{print strtonum("0x" $2)}')
        [ -z "$NM" ] && NM="?"
        BODY=$(awk -v s="<$S>:" 'index($0,s){f=1;next} f&&/^$/{exit} f' "$DIS")
        REAL=$(printf '%s\n' "$BODY" | grep -v 'nop\|xchg   %ax,%ax')
        INSNS=$(printf '%s\n' "$REAL" | grep -c ':')
        NETBY=$(printf '%s\n' "$BODY" | python3 -c '
import sys,re
addrs=[]
for l in sys.stdin.read().split("\n"):
    m=re.match(r"\s*([0-9a-f]+):\t(.*)",l)
    if m: addrs.append((int(m.group(1),16), m.group(2)))
tot=0
for i,(a,t) in enumerate(addrs):
    nxt = addrs[i+1][0] if i+1<len(addrs) else None
    sz = 1 if nxt is None else nxt-a
    if "nop" in t or "xchg   %ax,%ax" in t: continue
    tot += sz
print(tot)')
        CMOV=$(printf '%s\n' "$REAL" | grep -c 'cmov')
        printf '%-10s %-4s %-4s %-14s %7s %7s %7s %5s\n' \
          "$CXX" "$OPT" "$V" "$S" "$NM" "$INSNS" "$NETBY" "$CMOV"
      done
      TTEXT=$(size -A "$OBJ" | awk '$1==".text"{print $2}')
      TINS=$(grep -c $'\t' "$DIS")
      TCMOV=$(grep -c 'cmov' "$DIS")
      TSYM=$(nm -C "$OBJ" | grep -icE ' (t|w) ')
      printf '%-10s %-4s %-4s %-14s %7s %7s %7s %5s  syms=%s\n' \
        "$CXX" "$OPT" "$V" "TU-TOTAL" "$TTEXT" "$TINS" "-" "$TCMOV" "$TSYM"
    done
  done
done
