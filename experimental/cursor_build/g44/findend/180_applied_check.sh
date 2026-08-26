#!/bin/bash
# Confirms the header now in include/ generates the same code as the variant
# that won the comparison, and re-states the baseline next to it.
set -u
ROOT=/mnt/d/Data/LocalGit/boost
HERE=$ROOT/libs/container/experimental/cursor_build/g44/findend
EXP=$ROOT/libs/container/experimental
OUTDIR=$HERE/out/applied
mkdir -p "$OUTDIR"

printf '%-10s %-4s %-8s %8s %8s %6s\n' compiler opt which bytes insns cmov
for CXX in g++-16 clang++-22; do
  for OPT in -O2 -O3; do
    for W in base c applied; do
      case $W in
        base)    INC="-I$HERE/inc_base"; FLAG="" ;;
        c)       INC="-I$HERE/inc_c";    FLAG="-DFE_HAS_FLAG" ;;
        applied) INC="";                 FLAG="-DFE_HAS_FLAG" ;;
      esac
      OBJ=$OUTDIR/${CXX}_${OPT#-}_$W.o
      $CXX -std=c++17 $OPT -DNDEBUG -DFE_ENTRYPOINTS_ONLY $FLAG -c \
        $INC -I"$ROOT" -I"$EXP" "$HERE/110_codegen.cpp" -o "$OBJ" 2>"$OBJ.log" || {
        printf '%-10s %-4s %-8s BUILD FAILED\n' "$CXX" "$OPT" "$W"; head -20 "$OBJ.log"; continue; }
      objdump -d --no-show-raw-insn -C "$OBJ" > "$OBJ.dis"
      B=$(size -A "$OBJ" | awk '$1 ~ /^\.text/ {t+=$2} END{print t+0}')
      I=$(grep -c $'\t' "$OBJ.dis")
      M=$(grep -c 'cmov' "$OBJ.dis")
      printf '%-10s %-4s %-8s %8s %8s %6s\n' "$CXX" "$OPT" "$W" "$B" "$I" "$M"
    done
  done
done
