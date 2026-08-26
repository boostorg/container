#!/bin/bash
# Whole-TU totals, invariant to inlining decisions.
#   full  : leaf/walker probes + public entry points
#   entry : public entry points only, so the baseline is not charged for the
#           compound its real callers never evaluate
# Bytes are summed over every executable section, because clang emits each
# template instantiation into its own .text.<mangled> section and a plain
# ".text" row would miss all of them.
set -u

ROOT=/mnt/d/Data/LocalGit/boost
HERE=$ROOT/libs/container/experimental/cursor_build/g44/findend
EXP=$ROOT/libs/container/experimental
OUTDIR=$HERE/out/totals
mkdir -p "$OUTDIR"

python3 "$HERE/100_mkvariants.py" >/dev/null || exit 1

VARIANTS="base a b c d e"

printf '%-10s %-4s %-6s %-5s %8s %8s %6s %6s\n' \
   compiler opt tu var bytes insns cmov syms

for CXX in g++-16 clang++-22; do
  for OPT in -O2 -O3; do
    for TU in full entry; do
      EXTRA=""
      [ "$TU" = "entry" ] && EXTRA="-DFE_ENTRYPOINTS_ONLY"
      for V in $VARIANTS; do
        FLAG=""
        [ "$V" != "base" ] && FLAG="-DFE_HAS_FLAG"
        OBJ=$OUTDIR/${CXX}_${OPT#-}_${TU}_$V.o
        LOG=$OBJ.log
        if ! $CXX -std=c++17 $OPT -DNDEBUG $FLAG $EXTRA -c \
          -I"$HERE/inc_$V" -I"$ROOT" -I"$EXP" \
          "$HERE/110_codegen.cpp" -o "$OBJ" 2>"$LOG"; then
          printf '%-10s %-4s %-6s %-5s BUILD FAILED\n' "$CXX" "$OPT" "$TU" "$V"
          head -20 "$LOG"; continue
        fi
        DIS=$OBJ.dis
        objdump -d --no-show-raw-insn -C "$OBJ" > "$DIS"
        BYTES=$(size -A "$OBJ" | awk '$1 ~ /^\.text/ {t+=$2} END{print t+0}')
        INSNS=$(grep -c $'\t' "$DIS")
        CMOV=$(grep -c 'cmov' "$DIS")
        SYMS=$(nm "$OBJ" | grep -cE ' [tTwW] ')
        printf '%-10s %-4s %-6s %-5s %8s %8s %6s %6s\n' \
          "$CXX" "$OPT" "$TU" "$V" "$BYTES" "$INSNS" "$CMOV" "$SYMS"
      done
      echo
    done
  done
done
