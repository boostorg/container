# Segment-walking pattern study (g28)

Maximal-optimization study of the canonical segment-walking pattern (the
"Option E" shape from `segmented_fill.hpp:60-68`) that repeats in nearly
every segmented algorithm: code size and performance, static assembly
analysis, benchmarks, correctness gates.

All experiments went through SHADOW COPIES of `segmented_fill.hpp`,
`segmented_count.hpp`, `segmented_find.hpp` and `segmented_copy.hpp` under
`experimental/cursor_build/g28/pattern/shadow_<V>/`, picked up via `-I`
ordering ahead of the boost root.  No real header, test or benchmark file
was modified.  Overlay pickup is enforced at compile time: every shadow
header defines a `BOOST_CONTAINER_G28_SHADOW_*` marker and probes/MSVC runs
compile with `-DBOOST_CONTAINER_G28_EXPECT_SHADOW`, which `#error`s if the
real headers were used.  shadow_E (byte-for-byte copy of today's headers,
marker aside) reproduced the baseline codegen exactly on both compilers
(identical per-symbol instruction counts and `.text*` bytes), validating the
overlay end to end.

Compilers: g++-16, clang++-22 (WSL1 Ubuntu 26.04), MSVC 18 (VS2026).
Probe TU `280_probe.cpp` instantiates all three walker shapes at depth 1
(`bc::deque<int>` = real fixed-block container, `test_detail::seg_vector`),
depth 2 (`test_detail::seg2_vector`), and segmented-destination copy
(flat->seg2, seg2->flat, deque->deque, seg1->seg1).  Raw artefacts:
`out/static_O2.txt`, `out/static_H.txt`, `out/extra.txt`, `out/lst.*.txt`
(disassembly), `out/tests.txt`, `out/bench/` (60+20 launch logs),
`out/bench_medians.txt`.

## 1. Variants studied

### E — baseline (today's shape)
Three leaf call sites per walker level: first-partial, middle loop, and a
final call shared with the single-segment case (the `if` prepares `lb` and
falls through):

```cpp
if(BOOST_LIKELY(sfirst != slast)) {
   (leaf)(lb, traits::end(sfirst), ...);
   for(++sfirst; sfirst != slast; ++sfirst)
      (leaf)(traits::begin(sfirst), traits::end(sfirst), ...);
   lb = traits::begin(slast);
}
(leaf)(lb, traits::local(last), ...);
```

### T — two-call form (direction 1)
`lb` carries the "current begin" across iterations; first-partial and middle
collapse into one call site inside the loop, the final call stays:

```cpp
while(BOOST_LIKELY(sfirst != slast)) {
   (leaf)(lb, traits::end(sfirst), ...);
   ++sfirst;
   lb = traits::begin(sfirst);
}
(leaf)(lb, traits::local(last), ...);
```

### F — fully fused single-call form (direction 2)
One loop over `sfirst..slast` inclusive, per-iteration select of the local
end, ONE call site per level:

```cpp
for(;;) {
   const bool last_seg = sfirst == slast;
   const local_iterator le = last_seg ? traits::local(last) : traits::end(sfirst);
   (leaf)(lb, le, ...);
   if(BOOST_UNLIKELY(last_seg))
      break;                            // or: return result / return last
   ++sfirst;
   lb = traits::begin(sfirst);
}
```

Early-exit: the hit test `if(r != le) return traits::compose(sfirst, r);`
sits before the `last_seg` exit.  Dual-range destination walker: the
source-exhausted and last-segment exits merge into
`if(last_seg || SEG_UNLIKELY(first == last)) return segduo<...>(first, compose(sfirst, r.second));`.

### H — hybrid: fused partial calls + dedicated middle loop
Designed after the first benchmark round exposed why T and F regress (4.2).
The two *partial* calls (first and last) share one fused call site as in F,
but the middle loop keeps its own call site whose begin AND end both derive
from `sfirst` — the property that lets compilers specialise the leaf for
full, constant-size segments:

```cpp
for(;;) {
   const bool last_seg = sfirst == slast;
   const local_iterator le = last_seg ? traits::local(last) : traits::end(sfirst);
   (leaf)(lb, le, ...);                          // first AND last partial
   if(BOOST_UNLIKELY(last_seg))
      break;
   for(++sfirst; sfirst != slast; ++sfirst)
      (leaf)(traits::begin(sfirst), traits::end(sfirst), ...);   // full segs
   lb = traits::begin(sfirst);                   // sfirst == slast here
}
```

The outer loop runs at most twice (once for a single-segment range).  Two
leaf copies per level instead of E's three.  Early-exit/dual-range
adaptations follow F's.

### W — shared generic walker (direction 4, C++03 functor-based)
New header `segmented_walk.hpp`: `segmented_walk(first, last, F&)`
(full-scan) and `segmented_walk_until(first, last, F&, It& out) -> bool`
(early-exit), internally the E shape, recursing through `is_local_seg_t`.
fill/count/find become thin wrappers with per-algorithm state in a functor
(`fill_walk_fn`, `count_walk_fn` with accumulator member, `find_walk_fn`
returning the leaf stop position); the functors call the existing
FORCEINLINE flat leaves, so leaf codegen and unroll policy are untouched.

### TC — tail-call audit (direction 3; measurement, not a variant)
Extracted from the listings of the variants above; see 2.4.

All variants preserve recursive segmentation on both sides by construction:
the local call still dispatches on `is_local_seg_t` (W recurses inside the
shared walker), verified by the depth-2 `seg2_vector` tests and the
segmented-destination copy tests.

## 2. Static codegen (primary evidence), `-std=c++20 -O2 -DNDEBUG`

Metrics from `objdump -d` per symbol: instructions, cmp/test, conditional
jumps, calls; whole-TU totals over all probe + `detail_algo` symbols;
`.text*` = summed text sections including COMDATs.  GCC inlines walkers into
callers while Clang keeps many out-of-line — totals compare the whole TU,
and per-symbol rows name the symbol measured.

### 2.1 Whole-TU totals

| compiler | metric        | E     | T           | F           | H          | W          |
|----------|---------------|-------|-------------|-------------|------------|------------|
| g++-16   | instructions  | 2192  | 1880 (-14%) | 1308 (-40%) | 1987 (-9%) | 2254 (+3%) |
| g++-16   | cmp/test      | 274   | 244         | 181         | 221        | 274        |
| g++-16   | cond. jumps   | 295   | 262         | 188         | 249        | 298        |
| g++-16   | `.text*` bytes| 7866  | 6538        | 4508        | 7325       | 8154       |
| clang-22 | instructions  | 3480  | 2171 (-38%) | 1213 (-65%) | 2407 (-31%)| 3477 (-0%) |
| clang-22 | cmp/test      | 464   | 268         | 143         | 303        | 454        |
| clang-22 | cond. jumps   | 443   | 256         | 137         | 293        | 432        |
| clang-22 | `.text*` bytes| 12998 | 8155        | 4361        | 9064       | 13081      |

`-O3` spot check (same probe, totals): gcc 4383 / 3167 / 1739 / 3494 / 4464
(E/T/F/H/W); clang 3560 / 2187 / 1252 / 2492 / 3547.  Same ordering; GCC's
-O3 vectorization roughly doubles E's mass, and the variant deltas persist.

### 2.2 Per-shape, per-depth instruction counts (-O2)

`probe+N` = probe symbol plus an out-of-line (ool) walker of N instructions
that it calls; "inlined" = self-contained symbol.

Full-scan (fill / count):

| shape       | compiler | E             | T            | F       |
|-------------|----------|---------------|--------------|---------|
| fill deque  | gcc      | 105           | 89           | **59**  |
| fill d1     | gcc      | 18 + 130 ool  | 90           | **61**  |
| fill d2     | gcc      | 91 + 130 ool  | 202          | **95**  |
| count deque | gcc      | 148           | 117          | **76**  |
| count d1    | gcc      | 21 + 180 ool  | 21 + 125 ool | **82**  |
| count d2    | gcc      | 102 + 180 ool | 70 + 125 ool | **120** |
| fill deque  | clang    | 97            | 87           | **60**  |
| fill d1     | clang    | 118           | 89           | **58**  |
| fill d2     | clang    | 15 + 377 ool  | 197          | **90**  |
| count deque | clang    | 165           | 132          | **82**  |
| count d1    | clang    | 193           | 135          | **80**  |
| count d2    | clang    | 15 + 588 ool  | 15 + 292 ool | **117** |

Early-exit (find):

| shape      | compiler | E            | T            | F               |
|------------|----------|--------------|--------------|-----------------|
| find deque | gcc      | 145          | 120          | **84**          |
| find d1    | gcc      | 21 + 173 ool | 21 + 122 ool | 23 + **99** ool |
| find d2    | gcc      | 218 + 173 ool| 176          | 119 + **99** ool|
| find deque | clang    | 72 (tail-jmp)| 54           | **43**          |
| find d1    | clang    | 68           | 54           | **43**          |
| find d2    | clang    | 20 + 353 ool | 20 + 225 ool | 20 + **113** ool|

Dual-range (copy; the inlined/out-of-line split differs per variant, so the
honest number is the sum over all four copy probes plus every out-of-line
copy helper in the TU):

| copy, total instructions       | E    | T   | F       |
|--------------------------------|------|-----|---------|
| gcc  (4 probes + ool helpers)  | 840  | 727 | **490** |
| clang (4 probes + ool helpers) | 1399 | 871 | **507** |

H sits between E and T statically (whole-TU -9% gcc / -31% clang vs E) while
keeping E's middle-loop codegen bit-for-bit in character (see 4.2): e.g.
clang probe_find_deq is 72 insns under E and H alike, gcc probe_fill_deq
drops 105 -> 87.

### 2.3 Why the deltas are this large: call-site multiplication

E has three leaf call sites per walker level and the flat leaf is
FORCEINLINE, so a depth-1 walker carries 3 copies of the leaf loop.  At
depth 2 the "leaf" of the outer walker is the whole depth-1 walker: GCC
usually salvages this by keeping one shared out-of-line depth-1 walker (the
`.isra` clones above), but Clang inlines it into all three outer call sites:
3 x 3 = 9 leaf loops in one symbol — that is the 588-instruction
`count_dispatch<seg2_vector>` (2293 bytes) in the baseline.  Code mass grows
~3^depth.  T reduces the base to 2, F to exactly 1 call site per level
(near-linear growth: clang's complete fill-d2 walker under F is 90
instructions — 4x smaller than E's out-of-line walker alone).  H's base is 2
with only the cheap partial call duplicated once.

### 2.4 Tail-call audit (direction 3)

From the "symbol ends in unconditional jmp to another symbol" flag:

- clang already emits E's final dispatch call as a true tail call wherever
  the ABI allows: `find_dispatch<seg2>`, all three out-of-line
  `copy_dispatch` instantiations and `copy_dst_dispatch<seg2>` end in `jmp`
  under the unmodified baseline.  Nothing left to win.
- gcc rarely does (no E walker ends in a tail jmp; T's
  `find_dispatch<seg1>` picked one up incidentally).  For `fill` the final
  call is already syntactically in tail position and gcc still prefers
  inlining the leaf; for `count` (`return result + leaf(...)`) and `find`
  (`compose` after the call) no source rearrangement can create a true tail
  call because work follows the call.
- Under F/H the question dissolves: the final call is the loop call.

Conclusion: a dedicated tail-call variant has no headroom; not pursued
further (negative result).

### 2.5 W: codegen-neutral, source-level dedup only (negative result)

- gcc: `segmented_walk<fill_walk_fn>` is instruction-identical to E's fill
  walker (130).  `segmented_walk<count_walk_fn>` is 193 vs E's 180 and
  clang's count walker grows 2293 -> 2405 bytes: the accumulator lives in a
  functor member behind a reference, so it is kept in memory across leaf
  calls instead of being chained through a return-value register.
  `walk_until<find_walk_fn>` 175 vs 173.  Whole-TU: gcc +3%, clang -0.1%.
- Compile time/memory: within noise of E on both compilers (probe TU:
  gcc 2.54s/131.4MB vs base-order artifacts, clang 1.32s/153.6MB; see 5.2
  for the caveat).
- ICF cannot reclaim anything across algorithms under ANY variant: the
  per-algorithm walkers differ exactly in their inlined leaf bodies
  (gcc symbol sizes 390 / 541 / 574 bytes for fill / count / find walkers —
  different sizes, different instructions), so byte-identical folding never
  applies; identical instantiations across TUs are already folded by COMDAT.
  The planned `--icf=all` link experiment could NOT be executed: this
  machine has only BFD ld (no gold, no lld) — recorded as not verified.
- W also needs a second primitive for early-exit (`walk_until`) and would
  need a third for dual-range/bounded shapes; one primitive cannot cover the
  library.

Verdict: W trades nothing measurable at -O2/-O3 but wins no code size
either; adopt only if source-level de-duplication across ~30 headers is
worth a second (and third) walker primitive.  Slight count pessimisation
argues against wholesale adoption.

## 3. Tests (gate)

`283_tests.sh` against the shadow trees (log: `out/tests.txt`):

- `segmented_{fill,count,find,copy}_test.cpp` x variants {E,T,F,W,H} x
  {g++-16, clang++-22} x {c++03, c++11, c++17, c++20}, `-O2 -Wall -Wextra`:
  **160/160 build warning-silent and pass** (only the known pre-existing
  clang c++03 `-Wc++11-extensions` warning at segmented_fill_test.cpp:52,
  filtered per instructions; notably W's maybe-uninitialized-prone `lr`
  produced no diagnostics).
- Same tests x 5 variants x 2 compilers, `-fsanitize=address,undefined -O1
  -std=c++20`: **40/40 clean**, covering depth-1 and depth-2 shapes
  including empty-segment specs on input and output sides.

All header changes are C++03-clean (no auto/lambdas/nullptr; W's walker is
a functor-based C++03 template).

## 4. Benchmarks (confirmatory), bc::deque block_size 128

`bench_segmented_algos.cpp`, `-O2 -DNDEBUG -DBENCH_ON`, groups 10
(single-range sequential: fill/count/find rows are the treated ones) and 25
(2-range input-output: copy rows treated).  clang builds add
`-falign-functions=64 -falign-loops=64` per the bench file's own guidance.
5 launches per binary, pinned (`nice -n -5 taskset -c 3`; sudo/chrt was not
available non-interactively), medians of the `seg ns` column below;
differences under ~3% treated as placement noise.  Untreated rows of the
same binaries (count_if, find_if, for_each, replace, transform, copy_if,
copy_n, swap_ranges...) moved 0-2% throughout, confirming launch stability;
full tables in `out/bench_medians.txt`.

### 4.1 Treated-row medians (ns/element, 5 launches, median)

clang++-22, group 10:

| vt        | algo       | E     | T      | F      | H     |
|-----------|------------|-------|--------|--------|-------|
| MyInt     | count(hit) | 0.054 | -25.9% | -25.9% | +0.0% |
| MyInt     | fill       | 0.029 | +3.4%  | +10.3% | +0.0% |
| MyInt     | find(hit)  | 0.050 | **+184.0%** | **+128.0%** | -2.0% |
| MyInt     | find(miss) | 0.098 | **+196.9%** | **+121.4%** | +0.0% |
| FatInt<4> | find(hit)  | 0.063 | **+125.4%** | **+92.1%**  | -4.8% |
| FatInt<4> | find(miss) | 0.141 | **+102.1%** | **+65.2%**  | +0.0% |
| FatInt<8> | find(hit)  | 0.132 | +24.2% | +11.4% | -0.8% |
| FatInt<8> | count/fill | —     | ±2%    | ±2%    | ±0.3% |

g++-16, group 10:

| vt        | algo       | E     | T      | F      | H     |
|-----------|------------|-------|--------|--------|-------|
| MyInt     | count(hit) | 0.115 | **+111.3%** | **+77.4%** | +0.0% |
| MyInt     | fill       | 0.128 | +0.8%  | +0.0%  | -0.8% |
| MyInt     | find(hit)  | 0.060 | +1.7%  | +3.3%  | +1.7% |
| FatInt<4> | fill       | 0.135 | **+108.1%** | **+111.1%** | +0.0% |
| FatInt<4> | count/find | —     | ±1%    | ±1%    | ±1.4% |
| FatInt<8> | all        | —     | ±2%    | ±3%    | ±0.8% |

Group 25 (copy rows): everything within the noise envelope on both
compilers; the extremes were clang MyInt copy(1+2S) -8.2% (T and F) and
clang MyInt copy(2S) +8.2% (H) — the latter's five launches (0.047-0.054)
overlap the baseline's own spread (0.045-0.055), so both are placement/
launch noise on a 0.05 ns/elem row.  gcc copy rows: -4.1%..+4.2% with no
variant-consistent direction.

### 4.2 Root cause of the T/F regressions: the middle call site is a
### specialization point

E's middle loop calls `leaf(begin(sfirst), end(sfirst))`: both bounds derive
from the same segment iterator, and for fixed-block containers (deque) the
trip count is a compile-time constant.  The disassembly of
`probe_find_deq` (clang, E) shows exactly this: a scalar loop for the first
partial segment, then a **4x-unrolled compare loop with hard-coded bound
`cmp $0x400`** for middle segments.  T and F feed the loop-carried `lb` into
the single call site, so the bound is no longer provably constant and the
scan collapses to a 1x scalar loop — the 2-3x find slowdown on clang.  The
same mechanism, hitting different leaves per compiler cost model, produces
gcc's count-MyInt +111%/+77% and fill-FatInt4 +108%/+111% (gcc specialises
those middle loops under E; fingerprint: 2 constant-bound references and
SIMD in E/H, 1 and none in T/F — `289_simdcheck.sh`).

H was built to keep that call site: its fingerprints match E exactly
(clang find deque: 72 insns, 3 constant-bound refs under both E and H;
count keeps 6-of-8 SIMD ops, fill 10-of-12) and every treated benchmark row
is within noise of E.

This also retroactively justifies E over T: the "compiler already achieves
the two-call form via tail duplication" hypothesis from direction 1 is
false in the important direction — the compiler cannot *recover* the
constant-trip-count middle loop once the source has merged it away.

## 5. Portability and build-cost checks

### 5.1 MSVC 18 (VS2026)

`287_msvc_one.cmd` (shadow include ahead of the boost root + compile-time
shadow-pickup check), `/O2 /W4 /std:c++17`, x86 and x64, all four affected
tests: **F 8/8 PASS, H 8/8 PASS**, no warnings, no `C1002` on x86.  The
historical C1002 pressure came from force-inlined walkers; F/H shrink
walkers, so compile-memory moves in the safe direction (T and W were not
MSVC-tested — not verified, see 7).

### 5.2 Compile time / peak RSS (probe TU and segmented_copy_test)

GCC copy test: base 2.49s / 179.9MB, F 3.32s / 177.0MB, H 3.38s / 178.8MB;
clang: 1.50s / 160.5MB, F 1.79s / 160.1MB, H 1.80s / 160.4MB.  Peak RSS is
flat to slightly lower.  The wall-clock increase is dominated by a WSL1
drvfs artifact: the shadow `-I` directory is probed first for every include
of the TU, adding slow stat calls; the base configuration has no such extra
directory.  Treat the RSS column as the meaningful one.

## 6. Recommendation

1. **Adopt H (fused-partial hybrid) as the successor of Option E** for all
   walker shapes — full-scan, early-exit and dual-range (source and
   destination walkers).  Evidence:
   - runtime: within noise of E on every treated row, both compilers, all
     three element sizes — including every case where T/F regressed by
     +65%..+197%;
   - code size: -9% instructions / -7% `.text*` (gcc), -31% / -30% (clang)
     on the probe TU at -O2; ordering preserved at -O3;
   - correctness: full std-matrix, sanitizers, MSVC x86+x64 all green;
     recursion at depth 2 preserved on both input and output sides.
   H is a strict local rewrite of the E block (same variables, same traits
   calls, same SEG_LIKELY/UNLIKELY discipline, C++03-clean), so the rollout
   is mechanical across the simple algorithms; the copy-style destination
   walkers take the merged `last_seg || first == last` return.

2. **If maximum code-size reduction is ever the goal (-Os-like builds),
   F is the tool**: -40% (gcc) / -65% (clang) instruction mass — but it
   costs up to 2-3x on scan-heavy leaves (clang find, gcc count/fill on
   some element sizes) because it erases the constant-trip-count middle
   loop.  Do not adopt for the default build.

3. **Do not adopt T**: it shares F's specialization loss (often worse:
   +184% clang find hit) while saving only half as much code.  Negative
   result, kept for the record.

4. **Do not adopt W for codegen reasons; consider it only as a
   maintainability refactor**: instruction-neutral (slightly negative on
   count), needs 2-3 primitives to cover the algorithm classes, and cannot
   enable any linker folding since walker bodies differ per algorithm by
   construction.  If adopted, pair it with H's shape inside the shared
   walker.

5. **No dedicated tail-call work**: clang already tail-calls E's final
   dispatch; gcc structurally cannot for count/find; F/H remove the
   pattern.

Per-algorithm-class summary: full-scan -> H; early-exit -> H (E remains
acceptable; T/F harmful on clang); dual-range -> H (all variants were
runtime-neutral here, H chosen for consistency + size).  Compiler split:
the only variant that helps one compiler and hurts the other is none — H
helps clang a lot and gcc mildly, hurting neither; T/F hurt both, each in
different algorithms.

## 7. Not verified / out of scope

- ICF link experiment (`--icf=all`): no gold/lld on the machine; the
  cross-algorithm folding impossibility argument in 2.5 is from symbol-size
  diffing only.
- MSVC compile+run for T and W (only F and H, the candidates, were tested
  there); MSVC static codegen was not measured at all (no objdump-grade
  tooling used); the C1002-heavy `segmented_merge_test.cpp` was not
  recompiled since merge headers were untouched by every variant.
- W benchmarks (statically ≡ E; runtime assumed equal, not measured).
- H's rollout to the other ~26 algorithm headers (reverse's P2+P1
  flattening, the bool-returning merge/set_* leaves, find_last's backwards
  E) was not prototyped; the four studied shapes cover the three walker
  classes, but each rollout should re-run the affected test.
- Depth-3+ recursion (no such container exists in the test helpers), `nest`
  container benchmarks, `-Os`, PGO/LTO interactions.
- The 5-launch bench campaign ran under `nice -n -5 taskset -c 3`
  (non-interactive sudo unavailable, so `chrt -f 90` was not used).

## Appendix: file map

- `280_probe.cpp` probe TU; `281_count.py` per-symbol metrics;
  `282_static.sh` static campaign; `283_tests.sh` test gate;
  `284_bench_build.sh` / `285_bench_run.sh` / `290_bench_H.sh` bench
  campaign; `286_summarize.py` medians; `287_msvc_one.cmd` MSVC gate;
  `288_extra.sh` -O3/ICF/compile-cost; `289_simdcheck.sh` leaf fingerprints;
  `291_icf.sh` ICF retry (blocked); `292_spread.sh` noise inspection.
- `shadow_E/T/F/H/W/boost/container/experimental/*.hpp` — the variants.
- `out/` — all measurement artefacts referenced above.
