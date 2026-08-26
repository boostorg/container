#!/bin/bash
set -u
BOOSTROOT=/mnt/d/Data/LocalGit/boost
EXP=$BOOSTROOT/libs/container/experimental
G34=$EXP/cursor_build/g34/unroll
SNAP=$G34/snap
PROTO=$G34/snap_proto
OUT=$G34/runs

rm -rf $PROTO; cp -r $SNAP $PROTO
python3 - "$PROTO/boost/container/experimental/segmented_copy_if.hpp" <<'PYEOF'
import sys, io
p = sys.argv[1]
s = io.open(p, encoding='utf-8', newline='').read().replace('\r\n', '\n')

# --- P2: 4x manual source unroll of the generic (non-segmented destination) leaf ---
old_generic = """   if(BOOST_UNLIKELY(dst_first == dst_last))
      return segduo<SrcIter, DstIter>(first, dst_first);

   BOOST_CONTAINER_SEGMENTED_UNROLL(4)
   for(; first != last; ++first) {
      if(pred(*first)) {
         *dst_first = *first;
         ++dst_first;
         if(BOOST_UNLIKELY(dst_first == dst_last)) {
            ++first;
            goto out_path;
         }
      }
   }
   out_path:
   return segduo<SrcIter, DstIter>(first, dst_first);"""
new_generic = """   if(BOOST_UNLIKELY(dst_first == dst_last))
      return segduo<SrcIter, DstIter>(first, dst_first);

   #define BOOST_CONTAINER_SEG_COPY_IF_STEP()          \\
      if(pred(*first)) {                               \\
         *dst_first = *first;                          \\
         ++dst_first;                                  \\
         if(BOOST_UNLIKELY(dst_first == dst_last)) {   \\
            ++first;                                   \\
            goto out_path;                             \\
         }                                             \\
      }                                                \\
      ++first;

   while(first != last) {
      BOOST_CONTAINER_SEG_COPY_IF_STEP()
      if(first == last) break;
      BOOST_CONTAINER_SEG_COPY_IF_STEP()
      if(first == last) break;
      BOOST_CONTAINER_SEG_COPY_IF_STEP()
      if(first == last) break;
      BOOST_CONTAINER_SEG_COPY_IF_STEP()
   }
   #undef BOOST_CONTAINER_SEG_COPY_IF_STEP
   out_path:
   return segduo<SrcIter, DstIter>(first, dst_first);"""
assert old_generic in s, "generic leaf pattern not found"
s = s.replace(old_generic, new_generic)

# --- P1: 4x manual source unroll of the dual-random-access block loop ---
old_blocks = """      avail -= block_size;
      BOOST_CONTAINER_SEGMENTED_AUTO_UNROLL
      for(Diff chunk = block_size; chunk; ) {
         --chunk;
         if(pred(*cur)) {
            *dst_cur = *cur;
            ++dst_cur;
         }
         ++cur;
      }"""
new_blocks = """      avail -= block_size;
      Diff chunk = block_size;
      while(chunk >= 4) {
         chunk -= 4;
         if(pred(*cur)) { *dst_cur = *cur; ++dst_cur; }
         ++cur;
         if(pred(*cur)) { *dst_cur = *cur; ++dst_cur; }
         ++cur;
         if(pred(*cur)) { *dst_cur = *cur; ++dst_cur; }
         ++cur;
         if(pred(*cur)) { *dst_cur = *cur; ++dst_cur; }
         ++cur;
      }
      while(chunk) {
         --chunk;
         if(pred(*cur)) { *dst_cur = *cur; ++dst_cur; }
         ++cur;
      }"""
assert old_blocks in s, "block loop pattern not found"
s = s.replace(old_blocks, new_blocks)
io.open(p, 'w', encoding='utf-8', newline='').write(s)
print("prototype patch applied OK")
PYEOF
[ $? -ne 0 ] && { echo "PATCH FAILED"; exit 1; }

FLAGS="-std=c++20 -O3 -DNDEBUG -DBENCH_ON -DBOOST_CONTAINER_BENCH_SEGMENTED_GROUP=25 -falign-functions=64 -falign-loops=64"
cd $G34/exp
echo "=== build prototype benches ==="
g++-16     $FLAGS -I$PROTO -I$BOOSTROOT bench_segmented_algos.cpp -o $G34/gcc_proto.elf   2>&1|head -8
clang++-22 $FLAGS -I$PROTO -I$BOOSTROOT bench_segmented_algos.cpp -o $G34/clang_proto.elf 2>&1|head -8
ls -l $G34/gcc_proto.elf $G34/clang_proto.elf

echo
echo "=== correctness: segmented_copy_if_test against prototype, both compilers ==="
cp $EXP/segmented_copy_if_test.cpp $EXP/segmented_test_helper.hpp $G34/exp/ 2>&1
md5sum $G34/exp/segmented_copy_if_test.cpp
for cc in g++-16 clang++-22; do
  for inc in $SNAP $PROTO; do
    n=$(basename $inc)
    $cc -std=c++20 -O2 -DNDEBUG -I$inc -I$BOOSTROOT segmented_copy_if_test.cpp -o /tmp/t_$$.elf 2>&1 | head -5
    if [ -x /tmp/t_$$.elf ]; then /tmp/t_$$.elf > /dev/null 2>&1; echo "  $cc / $n : test exit=$?"; else echo "  $cc / $n : BUILD FAILED"; fi
    rm -f /tmp/t_$$.elf
  done
done

echo
for cfg in gcc_proto clang_proto; do
  echo "=== running $cfg x5 ==="
  for i in 1 2 3 4 5; do taskset -c 3 $G34/$cfg.elf > $OUT/$cfg.$i.txt 2>&1; done
done

med() { sort -g | awk '{a[NR]=$1} END{print a[int((NR+1)/2)]}'; }
for cfg in gcc_proto clang_proto; do
  echo
  echo "############ MEDIAN OF 5: $cfg ############"
  printf '%-28s%12s%12s%12s%11s%11s%11s\n' '< algo >' '<nsg/seg>' '<std/seg>' '<std/nsg>' '<seg ns>' '<std ns>' '<nsg ns>'
  grep -hE '^(copy|remove|swap|transform)' $OUT/$cfg.1.txt | cut -c1-28 | while IFS= read -r lab; do
    key=$(echo "$lab" | sed 's/ *$//')
    s=$(for i in 1 2 3 4 5; do grep -F "$key " $OUT/$cfg.$i.txt|head -1|awk '{print $(NF-2)}'; done|med)
    t=$(for i in 1 2 3 4 5; do grep -F "$key " $OUT/$cfg.$i.txt|head -1|awk '{print $(NF-1)}'; done|med)
    n=$(for i in 1 2 3 4 5; do grep -F "$key " $OUT/$cfg.$i.txt|head -1|awk '{print $NF}'; done|med)
    awk -v l="$key" -v s="$s" -v t="$t" -v n="$n" 'BEGIN{printf "%-28s%12.2f%12.2f%12.2f%11.3f%11.3f%11.3f\n",l,n/s,t/s,t/n,s,t,n}'
  done
done
echo DONE
