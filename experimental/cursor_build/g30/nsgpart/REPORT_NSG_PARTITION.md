# Why `nsg` is slower than `std::partition` (group 17, GCC), and how to fix it

Machine: AMD Ryzen 9 9950X (Zen 5), WSL1 Ubuntu 26.04, `g++-16` / `clang++-22`.
Benchmark: `bench_segmented_algos.cpp`, `-std=c++20 -O3 -I../../.. -DNDEBUG -DBENCH_ON
-DBOOST_CONTAINER_BENCH_SEGMENTED_GROUP=17 -falign-functions=64`.
Container: `bc::deque<MyInt, block_size<128>>`, N=100000, 5000 iterations.
Only `MyInt` rows exist: `main()` has the `MyFatInt<4>` / `MyFatInt<8>` calls commented
out, so the MyInt-vs-fat comparison **could not be run with the user's command** (see
"Not verified").

Column semantics (verified in source): `seg` = `bc::segmented_partition` on real deque
iterators (segmented dispatch, leaf `partition_disjoint_bidir_ranges`); `std` =
`std::partition` on the same deque iterators; `nsg` = `bc::segmented_partition` on
`wrapped_iterator<deque::iterator>` (default `Category = void` **preserves** the deque
iterator's `std::random_access_iterator_tag`, but the wrapper has no
`segmented_iterator_traits` specialization, so the primary template tags it
`non_segmented_iterator_tag`). `nsg` therefore runs the library's flat Hoare fallback
`segmented_partition_dispatch(BidirIt, BidirIt, Pred, non_segmented_iterator_tag, const Cat&)`
(`segmented_partition.hpp:97-116`) on random-access deque iterators.

## 1. User's exact command, partition rows (unpinned, single run: file `350_*_exact.txt`)

```
                              < nsg/seg >  < std/seg >  < std/nsg >  < seg ns >  < std ns >  < nsg ns >
gcc-16   partition(hit)          1.15         1.00         0.87        0.369       0.371       0.424
gcc-16   partition(miss)         1.69         1.24         0.73        0.219       0.271       0.370
clang-22 partition(hit)          1.20         1.22         1.02        0.304       0.370       0.364
clang-22 partition(miss)         1.25         1.38         1.11        0.257       0.356       0.320
```

## 2. Median of 5 pinned runs (`nice -n -5 taskset -c 3`; `sudo -n` unavailable)

| compiler | row | seg | std | nsg | nsg/std |
|---|---|---|---|---|---|
| gcc-16 | partition(hit) | 0.373 | 0.372 | 0.427 | **1.15** |
| gcc-16 | partition(miss) | 0.219 | 0.271 | 0.387 | **1.43** |
| clang-22 | partition(hit) | 0.301 | 0.365 | 0.360 | 0.99 |
| clang-22 | partition(miss) | 0.258 | 0.341 | 0.325 | 0.95 |

nsg run spreads are ≤1.5% (e.g. gcc miss: 0.381–0.388), so the gaps are stable and
far above the ~3% placement-noise floor. **The `nsg < std` symptom is GCC-only**; on
clang `nsg` already matches or beats `std`.

## 3. Which specialisations actually run

Everything is force-inlined; each of the six timed kernels is one `measure_batch`
clone (the 8-way unrolled harness duplicates the algorithm body 8×). Symbols in
`350_gcc.elf` (demangled, abbreviated):

- std: `measure_batch<bench_ops::std_partition_batch<bc::deque<MyInt,void,deque_opt<0,128,void,false>>, is_odd|is_negative<MyInt>>, batch_reset<...>> [clone .isra.0]`
- nsg: `measure_batch<bench_ops::seg_partition_batch<..., is_odd|is_negative<MyInt>, true>, batch_reset<...>> [clone .isra.0]`
- seg: same with `false`; its algorithm body is *not* inlined here (walkers are not
  force-inlined) — the only loops inside its clone are the batch-reset `memcpy`/delete.

Hypothesis check (the "decisive" one from the brief): **killed**. libstdc++ 16 has
only forward and bidirectional `__partition` (`/usr/include/c++/16/bits/stl_algo.h:1388,1414`);
there is **no random-access variant**, so `std::partition` on the deque's
random-access iterators runs the *same guarded Hoare loop* as the library fallback.
Same algorithm class on both sides; pred applied once per element on both sides.

## 4. Instruction-level comparison (gcc, miss case = pure backward scan)

Hot loop, `std` (e.g. `0xfc60`) vs `nsg` (e.g. `0xea70`) — **identical instruction
sequence**, 8 instructions, 3 branches, one memory re-load of the segment base per
element (the deque iterator's `operator--` boundary test against the map slot):

```
std @fc60 (24 B)                      nsg @ea70 (26 B)
  sub    $0x4,%rax                      sub    $0x4,%rcx
  cmp    %rax,%rdx                      cmp    %rcx,%rax
  je     out                            je     out
  mov    (%rax),%ecx                    mov    (%rcx),%r8d      ; REX ⇒ +2 B
  test   %ecx,%ecx                      test   %r8d,%r8d
  js     pred_hit                       js     pred_hit
  cmp    %rax,(%rsi)                    cmp    %rcx,(%rdi)      ; map-slot deref
  jne    .-0x16                         jne    .-0x18
```

The only difference is **code placement** of the 8 unrolled copies
(`353_gcc_align.txt`):

- std backward-scan copies: `fc60 fcd0 100d0 10150 101d0 10240 103c0 10440` —
  **0 of 8 cross a 64-byte line**.
- nsg copies: `ea70 eab0 eb30 eb80 ebf0 ec30 ed80 edd0` — five sit at `mod64 = 48`
  with 26-byte bodies ⇒ **5 of 8 cross a 64-byte line**.

Why GCC leaves them misaligned: with the benchmark's
`#pragma GCC optimize("align-loops=32")` GCC emits `.p2align` with a bounded skip,
so loops end up only 16-byte aligned when 32-byte alignment would need too much
padding — where a copy lands is effectively a dice roll per build.

### Proof of mechanism (`355_alignbench.cpp`, `355_result.txt`)

The exact 26-byte loop (same registers, same map-slot layout, same 782×128 data) was
placed at controlled offsets inside a 64-byte line via `.p2align 6` + `.rept N nop`:

```
pad= 0 fit   0.252 ns/elem      pad=40 CROSS 0.460
pad= 8 fit   0.246              pad=44 CROSS 0.513
pad=16 fit   0.247              pad=48 CROSS 0.459   <- real nsg copies (mod64=48)
pad=36 fit   0.239              pad=52 CROSS 0.512
```

Fits ⇒ ~0.25 ns/elem; crosses ⇒ ~0.46–0.51 ns/elem (≈1.9×): a 1-taken-branch-per-
iteration loop needs two fetch windows per iteration when split. Weighted over the
real binary's copies: nsg = (5×0.46 + 3×0.25)/8 ≈ **0.38** (measured 0.387);
std = 8×0.25/8 ≈ 0.25–0.27 (measured 0.271). The whole miss-case gap is reproduced by
placement alone. The hit case (1.15×) is the same effect diluted: its forward loop,
backward loop, and swap tail all matter, and the crossing counts are less lopsided
(nsg bwd 5/8 vs std bwd 3/8 crossing; fwd 4/8 vs 5/8).

Negative control (`356`, shadow header v1): rewriting the fallback with libstdc++'s
exact control flow (`while(true)/else-if/break` instead of `do/goto` +
`SEG_UNLIKELY`) re-rolled the dice and got *worse* (gcc miss nsg 0.439, nsg/std 1.59)
— control-flow shape does not control placement.

## 5. Improvement proposals (ranked)

### P1 (prototyped, recommended): manual 2× unroll of both fallback scan loops

`shadow2/boost/container/experimental/segmented_partition.hpp` — only
`segmented_partition_dispatch(..., non_segmented_iterator_tag, const Cat&)` changes;
each scan is unrolled by hand so one backedge covers two elements. C++03-clean, no
pragmas, no `auto`/lambdas; pred still applied exactly once per element; walkers,
segmented recursion and the segmented leaf untouched.

```cpp
      while (pred(*first)) {
         if (BOOST_UNLIKELY(++first == last))  goto first_ret;
         if (!pred(*first))                                  break;
         if (BOOST_UNLIKELY(++first == last))  goto first_ret;
      }
      do {
         if (BOOST_UNLIKELY(first == --last))  goto first_ret;
         if (pred(*last))                                    break;
         if (BOOST_UNLIKELY(first == --last))  goto first_ret;
      } while (!pred(*last));
```

Rationale: the loop body grows to ~52 bytes (gcc emits a 15-instruction body handling
2 elements, see `357_gcc_partition_funcs.dis` @`ef90`), so it *always* spans two
64-byte lines — fetch cost becomes one window per element **independent of
placement**, and taken-branch rate halves (Zen 5 sustains ~1 taken branch/cycle).
Static layout confirms: v2 nsg loops are 52–84 B, all "crossing", yet fastest.

Median-of-5 pinned, same pipeline (`357_*`), `segmented_partition_test.cpp` passes on
both compilers with the shadow header:

| compiler | row | std | nsg base | nsg v2 | nsg change | nsg/std |
|---|---|---|---|---|---|---|
| gcc-16 | hit  | 0.375 | 0.427 | **0.270** | −37% | 1.15 → **0.72** |
| gcc-16 | miss | 0.237 | 0.387 | **0.237** | −39% | 1.43 → **1.00** |
| clang-22 | hit  | 0.362 | 0.360 | **0.262** | −27% | 0.99 → 0.72 |
| clang-22 | miss | 0.342 | 0.325 | **0.239** | −26% | 0.95 → 0.70 |

(std moved 0.271→0.237 on gcc-miss because *its* copies also re-rolled placement in
the new binary — further evidence of the placement sensitivity.) v2 spreads ≤2%
across runs, and it also makes the fallback faster than the current *segmented* hit
path (0.270 vs seg 0.362).

### P2 (not prototyped): apply the same 2× unroll to `partition_disjoint_bidir_ranges`

The segmented leaf has the same 1-branch-per-element two-pointer shape (7-insn inner
loop, register-held bounds). seg hit = 0.36 ns/elem vs 0.27 for the unrolled flat
loop suggests ~25% headroom for `partition(hit)`; `partition(miss)` (0.219) is closer
to the floor. Same C++03/no-pragma constraints hold; needs the same measurement gate.

### P3 (benchmark/build-side note, not a header change): make loop alignment sticky

The gaps in *any* variant of this micro-loop come from whether a ≤26-byte body gets
32-byte alignment. `align-loops=32` with GCC's default max-skip leaves half the
copies 16-aligned. Aligning small hot loops harder (e.g. `-falign-loops=32` with a
larger max-skip, or 64) in the *benchmark* would remove this measurement noise source
when comparing code shapes. Not a library fix; listed because it explains why
ratio columns for byte-identical loops can read 1.4.

### Dropped: blocked random-access leaf

`BOOST_CONTAINER_SEGMENTED_PARTITION_BLOCKED_LEAF` no longer exists in the tree
(checked; the macro cleanup removed it). For the *fallback* it would only replace the
`first==last` compare with a counter — the deque iterator's per-element map-slot
compare is not removable through the wrapper, so the ceiling is ~1 instruction per
element, far less than P1 delivers, at higher complexity.

## 6. Not verified / caveats

- No PMU under WSL1: no `perf` counters. Front-end attribution rests on the
  controlled-alignment microbenchmark (which reproduces both endpoints numerically),
  not on op-cache-miss counters; "two fetch windows per iteration" on Zen 5 is the
  inferred mechanism.
- `MyFatInt` rows are commented out in `main()`; the user's command produces only
  `MyInt` rows, so the branch-bound vs bandwidth-bound comparison was not measurable
  without editing the benchmark (out of scope by instruction).
- `sudo -n` is unavailable; pinning used `nice -n -5 taskset -c 3` (the `nice` part
  is refused by the kernel, `taskset` applies). Spreads were ≤2%, so this was
  sufficient.
- P1 numbers are from shadow-header builds of this benchmark on this machine;
  other predicates/element sizes not re-measured (only group 17 partition rows).

## Files

- `350_*`: baseline builds, exact + 5 pinned runs, full/partition disasm, symbols.
- `351/358_medians.py`, `352_loops.py`, `353_align.py`, `359_layout.py`: analyzers.
- `352_gcc_loops.txt`, `353_{gcc,clang}_align.txt`: loop bodies + alignment tables.
- `355_alignbench.cpp` + `355_result.txt`: placement-penalty proof.
- `356_*` + `shadow/`: v1 (libstdc++ shape) — negative control.
- `357_*` + `shadow2/`: v2 (manual 2× unroll) — proposed change, tests pass.
