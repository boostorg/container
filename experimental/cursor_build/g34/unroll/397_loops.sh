#!/bin/bash
set -u
G34=/mnt/d/Data/LocalGit/boost/libs/container/experimental/cursor_build/g34/unroll
D=$G34/dis

show() { # elf start stop title
  echo "=================== $4 ==================="
  objdump -d --start-address=0x$2 --stop-address=0x$3 --no-show-raw-insn $1 \
   | sed -n '/:\t/p' | sed 's/<_Z[^>]*>//g' | sed 's/[ \t]*$//'
  echo
}

show $G34/clang_off.elf 1ec70 1ecc8 "clang_off  copy_if(1S miss) NSG   inner loop @1ec80"
show $G34/clang_off.elf 172b0 17308 "clang_off  copy_if(1S hit)  NSG   inner loop @172c0"
show $G34/clang_off.elf 1ce70 1cef0 "clang_off  copy_if(1S miss) STD   inner loop @1ce80"
echo DONE
