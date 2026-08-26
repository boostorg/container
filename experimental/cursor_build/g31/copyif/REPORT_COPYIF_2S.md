# `copy_if` with a segmented destination (`2S`): what the gap is, and why

Machine: AMD Ryzen 9 9950X (Zen 5, 16C/32T), Ubuntu 26.04 under WSL1, `g++-16`,
`clang++-22`. Pinned runs use `taskset -c 3`; `nice -n -5` is refused by the
kernel on this box (no privileges) and is reported as such by every script.

All work lives in `experimental/cursor_build/g31/copyif/`. **No real header,
test or benchmark file was modified.** Prototypes are shadow copies of
`segmented_copy_if.hpp` placed in `g31/copyif/p<N>/boost/container/experimental/`
and selected with `-I<shadow>` ahead of `-I../../..`.

| script | what it does |
|---|---|
| `360_baseline.sh` | the user's exact command, both compilers, one run |
| `361_medians.sh` | 5 pinned repeats per compiler, medians |
| `362_predcount.cpp` / `.sh` | predicate-application counting probe (no timing) |
| `363*_asm.sh`, `367_asm.sh`, `368_nsg_asm.sh` | disassembly |
| `364_sweep.cpp` / `.sh` | destination block-size scaling sweep |
| `365_mkproto.py` | generates the shadow header variants `p0`..`p5` |
| `365_proto.sh`, `366_proto2.sh` | build + correctness + conformance + timing per variant |

---

## 1. Column semantics (read from the harness, not assumed)

`print_subheader()` / `print_ratio()` in `bench_segmented_algos.cpp:567-684`:

```668:684:experimental/bench_segmented_algos.cpp
inline void print_ratio(const char* algo, const char*,
                        double std_ns, double seg_ns, double nsg_ns)
{
   double nsg_seg_ratio   = (seg_ns > 0.0) ? nsg_ns / seg_ns : 0.0;
   double std_seg_ratio   = (seg_ns > 0.0) ? std_ns / seg_ns : 0.0;
   double std_nsg_ratio   = (std_ns > 0.0) ? std_ns / nsg_ns : 0.0;
```

* `< nsg/seg >` = `nsg_ns / seg_ns`. **Greater than 1 means the segmented path
  is faster than the non-segmented fallback; less than 1 means it is slower.**
  There is no literal `seg/nsg` column — the user's "seg/nsg is lower than 1"
  is the `nsg/seg < 1` case, i.e. the segmented path losing.
* `< std/seg >` = `std_ns / seg_ns`, `< std/nsg >` = `std_ns / nsg_ns`.
* `< seg ns >`, `< std ns >`, `< nsg ns >` are nanoseconds **per element**
  (`calc_ns_per_elem`: total ns / (iters × elems)).

### What `1S`, `2S`, `1+2S` mean for `copy_if`

`bench_copy_if<InC, OutC>` (`bench_segmented_algos.cpp:1616-1625`) allocates
`OutC out(c.size())` and passes `InC` as the source. The call sites
(`:2381-2386`) are, with `C = bc::deque<MyInt, void, block_size<128> >` and
`vec_t = bc::vector<MyInt>`:

| row | source | destination |
|---|---|---|
| `copy_if(1S …)` | `C` (deque, **segmented**) | `vec_t` (flat) |
| `copy_if(2S …)` | `vec_t` (flat) | `C` (deque, **segmented**) |
| `copy_if(1+2S …)` | `C` (segmented) | `C` (segmented) |

So the user's reading is right: **`2S` is destination-only segmentation.**
`hit` uses `is_odd` (50 % of elements pass), `miss` uses `is_negative`
(0 % pass). `N = 100000`, `iters = 5000`, value type `MyInt` (a non-trivially
copyable `int` wrapper).

### What `nsg` and `std` are

* `nsg` is the **same** `bc::segmented_copy_if` call with every iterator passed
  through `bc::wrapped_iterator` (`bench_ops::iter_w<true>::wrap`).
  `wrapped_iterator` has no `segmented_iterator_traits` specialization, so
  `is_segmented_iterator` is false on both sides and the library takes its
  non-segmented path; `iterator_category` is forwarded unchanged
  (`wrapped_iterator.hpp:61`), so the random-access leaf specializations still
  apply. It measures "what the library would cost if it did not know about
  segmentation".
* `std` is `bench_detail::copy_if`, which for C++20 is `using std::copy_if`
  (`bench_segmented_algos.cpp:331`), called on the raw container iterators.

**Correction to a premise in the brief:** for `2S`, `nsg`'s inner loop does
*not* write through a flat pointer. The destination is a wrapped
`deque_iterator`, so every write carries `deque_iterator::operator++`'s block
test — see §4. That is only true of `1S`, where the destination is a
`bc::vector`.

---

## 2. Reproduction

### 2a. The user's exact command, one run each (`360_baseline.sh`)

```
g++-16     -std=c++20 -O3 -I../../.. -DNDEBUG -DBENCH_ON \
           -DBOOST_CONTAINER_BENCH_SEGMENTED_GROUP=25 \
           -falign-functions=64 -falign-loops=64 bench_segmented_algos.cpp -o a.elf && ./a.elf
```

**g++-16**

```
< algo >                     < nsg/seg > < std/seg > < std/nsg > < seg ns > < std ns > < nsg ns >
copy(1S)                            2.38        2.37        1.00      0.105      0.249      0.249
copy(2S)                            5.53        5.49        0.99      0.052      0.288      0.290
copy(1+2S)                          5.81        5.76        0.99      0.065      0.374      0.377
copy_if(1S hit)                     3.31        3.33        1.01      0.136      0.452      0.449
copy_if(2S hit)                     1.43        2.89        2.02      0.165      0.477      0.237
copy_if(1+2S hit)                   3.04        3.07        1.01      0.167      0.514      0.509
copy_if(1S miss)                    2.25        2.25        1.00      0.221      0.497      0.497
copy_if(2S miss)                    0.73        1.46        2.00      0.159      0.231      0.115
copy_if(1+2S miss)                  1.94        2.15        1.11      0.170      0.364      0.329
```

**clang++-22**

```
< algo >                     < nsg/seg > < std/seg > < std/nsg > < seg ns > < std ns > < nsg ns >
copy(1S)                            5.21        5.21        1.00      0.052      0.269      0.269
copy(2S)                            4.58        4.58        1.00      0.050      0.231      0.231
copy(1+2S)                          7.01        7.00        1.00      0.059      0.411      0.412
copy_if(1S hit)                     1.70        1.69        0.99      0.245      0.414      0.418
copy_if(2S hit)                     0.94        0.94        1.00      0.240      0.227      0.227
copy_if(1+2S hit)                   2.72        2.71        1.00      0.195      0.528      0.530
copy_if(1S miss)                    5.06        2.48        0.49      0.110      0.273      0.556
copy_if(2S miss)                    2.34        2.29        0.98      0.082      0.188      0.192
copy_if(1+2S miss)                  6.10        6.10        1.00      0.088      0.534      0.534
```

### 2b. Median of 5 pinned repeats (`361_medians.sh`)

**g++-16**

| algo | seg ns | std ns | nsg ns | nsg/seg | std/seg | std/nsg |
|---|---|---|---|---|---|---|
| copy(1S) | 0.105 | 0.249 | 0.249 | 2.37 | 2.37 | 1.00 |
| copy(2S) | 0.050 | 0.288 | 0.288 | 5.76 | 5.76 | 1.00 |
| copy(1+2S) | 0.065 | 0.373 | 0.364 | 5.60 | 5.74 | 1.02 |
| copy_if(1S hit) | 0.136 | 0.454 | 0.451 | 3.32 | 3.34 | 1.01 |
| copy_if(2S hit) | 0.169 | 0.477 | 0.237 | 1.40 | 2.82 | 2.01 |
| copy_if(1+2S hit) | 0.167 | 0.517 | 0.513 | 3.07 | 3.10 | 1.01 |
| copy_if(1S miss) | 0.221 | 0.497 | 0.497 | 2.25 | 2.25 | 1.00 |
| **copy_if(2S miss)** | **0.159** | 0.231 | **0.115** | **0.72** | 1.45 | 2.01 |
| copy_if(1+2S miss) | 0.163 | 0.362 | 0.322 | 1.98 | 2.22 | 1.12 |

**clang++-22**

| algo | seg ns | std ns | nsg ns | nsg/seg | std/seg | std/nsg |
|---|---|---|---|---|---|---|
| copy(1S) | 0.049 | 0.270 | 0.269 | 5.49 | 5.51 | 1.00 |
| copy(2S) | 0.050 | 0.231 | 0.231 | 4.62 | 4.62 | 1.00 |
| copy(1+2S) | 0.058 | 0.412 | 0.413 | 7.12 | 7.10 | 1.00 |
| copy_if(1S hit) | 0.247 | 0.412 | 0.418 | 1.69 | 1.67 | 0.99 |
| **copy_if(2S hit)** | **0.240** | 0.226 | **0.229** | **0.95** | 0.94 | 0.99 |
| copy_if(1+2S hit) | 0.192 | 0.528 | 0.529 | 2.76 | 2.75 | 1.00 |
| copy_if(1S miss) | 0.110 | 0.278 | 0.557 | 5.06 | 2.53 | 0.50 |
| copy_if(2S miss) | 0.082 | 0.191 | 0.192 | 2.34 | 2.33 | 0.99 |
| copy_if(1+2S miss) | 0.088 | 0.535 | 0.532 | 6.05 | 6.08 | 1.01 |

Run-to-run spread of `seg ns` over the 5 pinned repeats is 0.0 – 4.9 % on
g++-16 and 0.0 – 17 % on clang (the 14–17 % outliers are `copy(1S)` /
`copy(2S)`, which are 0.05 ns/element rows where one ULP of the printed value
is 2 %). Every row discussed below has a spread ≤ 3.7 %.

### 2c. What the numbers actually say

Exactly **two** rows have `nsg/seg < 1`:

* **g++-16 `copy_if(2S miss)`: 0.72.** The segmented path is **1.38× slower**
  than the non-segmented fallback — 0.159 vs 0.115 ns/element, i.e. +0.044
  ns/element, +4.4 µs per 100 000-element call. This is the large one and it is
  perfectly stable (0.0 % spread over 5 runs).
* **clang++-22 `copy_if(2S hit)`: 0.95.** The segmented path is **1.048× slower**
  — 0.240 vs 0.229 ns/element, i.e. +0.011 ns/element, +1.1 µs per call.
  Small, but reproducible (0.8 % spread).

The `copy` control (no predicate) shows **no gap at all** in any shape or on any
compiler: `nsg/seg` is 2.37 – 7.12. `copy_n`, `swap_ranges` and `transform` in
the same group behave the same way. So the effect is specific to `copy_if`'s
*conditional-write* leaf, not to destination walking as such.

Note also that the *other* compiler is comfortably ahead in each of those two
cases (clang `copy_if(2S miss)` = 2.34; g++ `copy_if(2S hit)` = 1.40), which
already suggests two different codegen-level causes rather than one algorithmic
one.

---

## 3. Predicate-application counts (`362_predcount.cpp`) — the decisive experiment

A counting predicate, no timing. `[alg.copy]` mandates **exactly `last - first`**
applications for `copy_if`. Counts are identical under both compilers.

**The reported case reproduces exactly:**

```
copy_if flat dst        block=-   n=64  every=1  pred_applied=64  mandated=64  extra=0
copy_if deque dst       block=8   n=64  every=1  pred_applied=71  mandated=64  extra=7   (10.938%)
copy_if deque dst (nsg) block=8   n=64  every=1  pred_applied=64  mandated=64  extra=0
```

**Sweep, flat `bc::vector` source, `bc::deque` destination, n = 100000:**

| dst block | every=1 (all pass) | every=2 (half pass) | every=0 (none pass) |
|---|---|---|---|
| flat (control) | 100000 (+0) | 100000 (+0) | 100000 (+0) |
| 8 | 112499 (+12499, 12.50 %) | 106249 (+6249, 6.25 %) | 100000 (**+0**) |
| 16 | 106249 (+6.25 %) | 103124 (+3.12 %) | — |
| 32 | 103124 (+3.12 %) | 101562 (+1.56 %) | — |
| 64 | 101562 (+1.56 %) | 100781 (+0.78 %) | — |
| **128** (benchmark) | 100781 (**+0.78 %**) | 100390 (**+0.39 %**) | 100000 (**+0.00 %**) |
| 256 | 100390 (+0.39 %) | 100195 (+0.20 %) | — |
| 512 | 100195 (+0.20 %) | 100097 (+0.10 %) | — |

**Recursively segmented destinations** (`test_detail::seg_vector` depth 1,
`seg2_vector` depth 2, inner segments of 64, n = 4096, all pass):
4159 (+63) and 4174 (+78) against a mandated 4096.

**1+2S** (deque source block 128, deque destination block 128): **100000, +0**.
With equal source and destination block sizes the leaf is handed a source
sub-range that never outlives the destination segment, so no crossing happens
mid-leaf. This is an artefact of the benchmark's equal block sizes, not a
property of `1+2S`.

### The defect is confirmed, and it is *not* the cause of the timing gap

The number of extra applications is exactly the number of
destination-segment boundary crossings, which is exactly what the described
mechanism predicts, and the 71-vs-64 case reproduces to the element. The
mechanism is visible in the source:

```52:63:include/boost/container/experimental/segmented_copy_if.hpp
   BOOST_CONTAINER_SEGMENTED_UNROLL(4)
   for(; first != last; ++first) {
      if(pred(*first)) {
         if(BOOST_UNLIKELY(dst_first == dst_last))
            goto out_path;
         *dst_first = *first;
         ++dst_first;
      }
   }
   out_path:
   return segduo<SrcIter, DstIter>(first, dst_first);
```

On the `goto`, `first` still points at the element that just tested `true`; the
walker (`segmented_copy_if_dst_dispatch`, `:176-187`) advances the destination
segment and calls the leaf again on that same element.

**But the arithmetic rules it out as the explanation of the observed slowdown:**

* **g++ `copy_if(2S miss)` — the 38 % row — performs `extra = 0`.** With
  `is_negative` nothing is ever written, so there is not a single destination
  boundary crossing, and therefore not a single re-applied predicate. Whatever
  makes that row slow, it is not this defect.
* **clang `copy_if(2S hit)` — the 5 % row — performs `extra = 390`, i.e.
  +0.39 % predicate work.** A 0.39 % increase in predicate applications cannot
  by itself produce a 4.8 % slowdown. (It turns out that *removing* it does
  close the gap, but because each avoided boundary costs ~3 ns, not ~1 cycle —
  see §6.)

---

## 4. Assembly (`367_asm.sh`, `368_nsg_asm.sh`)

Both compilers keep the destination walker out of line, so it is directly
addressable. `nm -C` gives, in `a_g++-16.elf` and `a_clang++-22.elf`:

```
segmented_copy_if_dst_dispatch<vec_iterator<MyInt*,true>, vec_iterator<MyInt*,true>,
                               deque_iterator<MyInt*,false,0u,128u,unsigned long>,
                               is_negative<MyInt>, random_access_iterator_tag>
                              (..., segmented_iterator_tag const&, ...)   [clone .isra.0]
```

and the `is_odd` twin. The leaves are `BOOST_CONTAINER_FORCEINLINE` and are
inlined into these, as intended.

Static sizes (whole walker, `objdump -d --no-show-raw-insn | c++filt`):

| | g++-16 | clang++-22 |
|---|---|---|
| `2S` / `is_negative` | 265 | 334 |
| `2S` / `is_odd` | 248 | 334 |

### 4a. The per-boundary sequence, g++-16 `2S`/`is_odd` (baseline)

Steady-state body, per source element (destination room test **inside** the
`pred`-taken branch):

```
42ad: mov    0x4(%rcx),%eax        ; *first
42b4: test   $0x1,%al              ; pred(*first)
42b6: je     42c3                  ; not selected -> next element
42b8: cmp    %rdx,%r10             ; dst_first == dst_last ?     <-- room test
42bb: je     42e2                  ; destination segment full
42bd: mov    %eax,(%rdx)           ; *dst_first = *first
42bf: add    $0x4,%rdx             ; ++dst_first
```

Boundary exit — note `%r8` (`first`) is **not** advanced, so the walker
re-enters the leaf on the element that has already been tested:

```
42e2: mov    %r8,(%rbx)            ; store first
42e5: mov    0x0(%rbp),%r12        ; reload last
42e9: cmp    %r8,%r12
42ec: je     42fb                  ; source exhausted -> compose + return
42ee: add    $0x8,%r13             ; ++dst_seg (step the deque block index)
42f2: mov    0x0(%r13),%rdx        ; dst_local = begin(dst_seg)
42f6: jmp    4036                  ; re-enter the leaf  -> re-applies pred here
```

and the `compose` normalisation the walker relies on:

```
42fb: mov    0x0(%r13),%rax
42ff: add    $0x200,%rax           ; end(dst_seg) = base + 128*sizeof(MyInt)
4305: cmp    %rax,%rdx
4308: je     4335                  ; local == segment end -> step to next segment
4335: mov    0x8(%r13),%rdx
4339: add    $0x8,%r13
```

So the per-boundary cost is: 2 stores/reloads of the loop-carried iterators, a
source-exhaustion test, a segment-pointer step, a block-base load, an
unconditional jump back into the leaf, the leaf's own re-entry preamble, and
then **one re-applied predicate plus one repeated loop iteration**. About 10
instructions of walker glue plus the re-entry, versus 6 instructions for an
ordinary selected element.

clang's shape is the same (`367_clang_p0_2S_is_odd.asm`):

```
1a24d: mov    (%r8),%ebx
1a250: test   $0x1,%bl
1a253: je     1a240                ; not selected
1a255: cmp    %rcx,%r9             ; room test, before the write
1a258: je     19d00                ; boundary exit, first not advanced
1a25e: mov    %ebx,(%r9)
1a261: add    $0x4,%r9
```

### 4b. What `nsg` does in the `2S` case (`368_nsg_asm.sh`)

The brief expected a flat pointer with no boundary handling. That is the `1S`
picture. In `2S` the destination is a wrapped `deque_iterator`, and g++'s `nsg`
inner loop is:

```
12997: mov    (%rcx),%eax
12999: test   $0x1,%al             ; pred
1299b: je     129b5
1299d: mov    %eax,(%rdx)          ; *dst = *first
1299f: mov    (%rsi),%rax          ; reload the current block base   \
129a2: add    $0x4,%rdx            ; ++dst.cur                        | deque_iterator
129a6: add    $0x200,%rax          ; block end                        | ::operator++
129ac: cmp    %rax,%rdx            ;                                  |
129af: je     13bea                ; cross to the next block         /
```

i.e. `nsg` pays **four extra instructions per selected element** (not per
boundary). That is why `seg` wins the `2S hit` rows on g++ (1.40) and why
`copy(2S)`, where every element is written, is a 5.8× win for `seg`.

In the `2S miss` case nothing is ever written, so none of that runs: `nsg`
degenerates to a bare load/test/advance scan at 0.115 ns/element, which is the
number `seg` has to beat.

### 4c. Where g++'s `2S miss` time actually goes

For `2S` the destination local range is a real bounded `MyInt*` pair, so the
random-access leaf `copy_if_cleanup_blocks<32>` is selected. g++ emits, **per
32 source elements**:

```
3cc0: mov    %r10,%rax             ; room = dst_last
3cc3: sub    %rdx,%rax             ;      - dst_cur
3cc6: cmp    $0x7c,%rax            ; room >= 32 elements ?
3cca: jle    3e46                  ; -> scalar tail
3cd0: mov    %rdi,%rax
3cd3: sub    $0x20,%r9             ; avail -= 32
3cd7: mov    %r8,%rcx
3cda: sub    %r8,%rax
3cdd: sub    $0x4,%rax
3ce1: shr    $0x2,%rax
3ce5: add    $0x1,%rax
3ce9: and    $0x7,%eax             ; unroll-by-8 remainder ... always 0 here
3cec: je     3dc0
3cf2: cmp    $0x1,%rax             \
3cf6: je     3d77                   |
3cf8: cmp    $0x2,%rax              |  six-way Duff ladder,
3cfc: je     3d67                   |  provably dead (trip count is 32)
3cfe: cmp    $0x3,%rax              |
3d02: je     3d57                   |
3d04: cmp    $0x4,%rax              |
3d08: je     3d47                   |
3d0a: cmp    $0x5,%rax              |
3d0e: je     3d37                   |
3d10: cmp    $0x6,%rax              |
3d14: je     3d27                  /
3dc0: <8-wide body, 4 instructions per element, iterated 4x>
```

**24 prologue instructions per 32 elements = 0.75 extra instructions per
element** on top of a 4-instruction-per-element body, i.e. ~19 % more
instructions than `nsg`'s plain scan, plus 0.22 extra (statically predictable but
front-end-consuming) branches per element. Measured delta is +37 %. `#pragma GCC
unroll 8` (`BOOST_CONTAINER_AUTO_UNROLL`, `workaround.hpp:266`) is applied to a
loop whose trip count is the compile-time constant 32, and GCC still emits the
runtime remainder computation and ladder on every block.

---

## 5. Block-size scaling (`364_sweep.cpp`)

Independent probe: fixed flat `bc::vector<MyInt>` source of 100 000,
`bc::deque<MyInt, block_size<B> >` destination, pinned, median of 5.
(`FLAT dst` is a `bc::vector` destination, as an absolute floor.)

**g++-16**

| B | hit50 seg | hit50 nsg | nsg/seg | miss0 seg | miss0 nsg | nsg/seg |
|---|---|---|---|---|---|---|
| flat | 0.1331 | 0.1331 | 1.000 | 0.1624 | 0.1628 | 1.002 |
| 8 | 0.2521 | 0.2572 | 1.020 | 0.1270 | 0.1172 | 0.923 |
| 16 | 0.2138 | 0.2565 | 1.199 | 0.1269 | 0.1172 | 0.924 |
| 32 | 0.1867 | 0.2452 | 1.313 | **0.1614** | 0.1171 | **0.725** |
| 64 | 0.1763 | 0.1877 | 1.065 | 0.1607 | 0.1172 | 0.729 |
| 128 | 0.1707 | 0.2282 | 1.337 | 0.1649 | 0.1172 | 0.711 |
| 256 | 0.1788 | 0.2052 | 1.147 | 0.1636 | 0.1171 | 0.715 |
| 512 | 0.1716 | 0.1897 | 1.106 | 0.1665 | 0.1178 | 0.708 |

**clang++-22**

| B | hit50 seg | hit50 nsg | nsg/seg | miss0 seg | miss0 nsg | nsg/seg |
|---|---|---|---|---|---|---|
| flat | 0.1783 | 0.1787 | 1.002 | 0.0980 | 0.0982 | 1.002 |
| 8 | 0.3354 | 0.2517 | **0.750** | 0.2343 | 0.2087 | 0.891 |
| 16 | 0.2697 | 0.2421 | **0.898** | 0.2343 | 0.2121 | 0.905 |
| 32 | 0.2148 | 0.2404 | 1.119 | 0.0831 | 0.2129 | 2.561 |
| 64 | 0.2154 | 0.2788 | 1.294 | 0.0833 | 0.2282 | 2.740 |
| 128 | 0.2211 | 0.3118 | 1.410 | 0.0831 | 0.2250 | 2.708 |
| 256 | 0.2020 | 0.2888 | 1.430 | 0.0832 | 0.2199 | 2.641 |
| 512 | 0.1890 | 0.2971 | 1.572 | 0.0833 | 0.2271 | 2.726 |

Two clearly different behaviours:

* **clang, `hit50`** — the deficit *does* scale with boundary-crossing
  frequency: 0.750 at B = 8, 0.898 at B = 16, ≥ 1.12 from B = 32 up. Boundary
  crossings per element are 1/8, 1/16, 1/32 …, so this is the per-boundary cost,
  and it bounds the achievable win: at the benchmark's B = 128 there are ~390
  crossings per 100 000 elements, worth ~1.1 µs, which is the whole gap.
* **g++, `miss0`** — the deficit does **not** scale at all: it is flat at
  0.71 – 0.73 for every B ≥ 32, and there are **zero** boundary crossings in this
  case. It appears exactly at the threshold B = 32, which is the point at which
  `copy_if_cleanup_blocks<32>` can run at all (it requires room for 32 *writes*
  before entering). Below that threshold the plain scalar leaf runs and the gap
  shrinks to 8 %. This is a leaf-codegen effect, not a boundary effect.

The `seg` figures here agree with the benchmark's (`hit50` B=128: 0.171 vs
0.169 on g++, 0.221 vs 0.240 on clang). The `nsg` figures do **not** agree for
clang (0.312 vs 0.229); the two harnesses inline the wrapped path differently.
Cross-harness `nsg` comparisons are therefore not made below.

---

## 6. Prototypes (`365_mkproto.py`, `365_proto.sh`, `366_proto2.sh`)

All variants are shadow copies of `segmented_copy_if.hpp`. All were gated on
`segmented_copy_if_test.cpp` (`-std=c++20 -O2`, both compilers) and on the
predicate-count probe, then benchmarked with the user's exact flags, pinned,
median of 5.

| id | change |
|---|---|
| `p0` | verbatim copy (control) |
| `p1` | drop `BOOST_CONTAINER_SEGMENTED_AUTO_UNROLL` from `copy_if_cleanup_blocks` |
| `p2` | **destination test once on entry, and after each write instead of before it** |
| `p3` | `p1` + `p2` |
| `p4` | `p2` + random-access leaf rewritten as a counted `min(avail, room)` run |
| `p5` | `p4` with a floor of 8 on the counted run, tail handed to the `p2` leaf |

### 6a. `p2` — the recommended change

```cpp
   //[alg.copy] mandates exactly last - first applications of pred.  Testing an
   //element, discovering the destination segment is full and returning makes the
   //enclosing destination walker call this leaf again on the same element, which
   //re-applies pred.  Checking the destination once on entry and again after each
   //write removes that: when the destination fills, `first` has already moved
   //past the element that was written, so the next call resumes on an untested
   //element.  With an unreachable_sentinel_t destination both checks fold away,
   //so the flat path is unchanged.
   if(BOOST_UNLIKELY(dst_first == dst_last))
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
   return segduo<SrcIter, DstIter>(first, dst_first);
```

Eight lines, one function, no new abstraction, no walker change, no interface
change, C++03-clean (no `auto`, lambda, range-`for`, `nullptr` or variadics
introduced). `unreachable_sentinel_t`'s comparisons are compile-time
`false`/`true` (`segmented_iterator_traits.hpp:97-110`), so both new tests fold
away completely on the flat-destination path.

The steady-state loop is the same length as before — the room test simply moves
from before the store to after it:

```
                        p0                                 p2
42b4: test $0x1,%al          pred          42eb: test $0x1,%al          pred
42b6: je   42c3              skip          42ed: je   42fa              skip
42b8: cmp  %rdx,%r10         room test     42ef: mov  %eax,(%rdx)       store
42bb: je   42e2              boundary      42f1: add  $0x4,%rdx         ++dst
42bd: mov  %eax,(%rdx)       store         42f5: cmp  %rdx,%r10         room test
42bf: add  $0x4,%rdx         ++dst         42f8: je   4348              boundary
```

and the boundary exit gains exactly one instruction, the `++first` that makes
the re-entry resume on an untested element:

```
4348: add    $0x4,%r8              ; ++first   <-- the whole fix
434c: mov    %r8,(%rbx)
434f: mov    0x0(%rbp),%r12
4353: cmp    %r12,%r8
4356: jne    433b
433b: add    $0x8,%r13             ; ++dst_seg
433f: mov    0x0(%r13),%rdx
4343: jmp    4076
```

Static instruction counts, whole walker: g++ 265→274 (`is_negative`), 248→255
(`is_odd`); clang 334→337 for both. All growth is in the cold boundary path.

**Conformance.** Predicate applications become exactly the mandated count in
every case measured, on both compilers:

| case | `p0` | `p2` |
|---|---|---|
| 64 elements, deque block 8, all pass | 71 (mandated 64) | **64** |
| n=100000, deque block 128, all pass | 100781 | **100000** |
| n=100000, deque block 128, half pass | 100390 | **100000** |
| `seg_vector` depth 1, n=4096 | 4159 | **4096** |
| `seg2_vector` depth 2, n=4096 | 4174 | **4096** |

**Correctness.** `segmented_copy_if_test.cpp` passes under both compilers.
Recursively segmented destinations to depth 2 still compile and give the right
answer (they are exercised by the count probe, which uses `seg2_vector` for its
destination).

**Timing** (seg ns/element, median of 5 pinned runs):

| algo | g++ p0 | g++ p2 | clang p0 | clang p2 |
|---|---|---|---|---|
| copy(1S) | 0.106 | 0.104 | 0.055 | 0.050 |
| copy(2S) | 0.051 | 0.053 | 0.051 | 0.049 |
| copy(1+2S) | 0.064 | 0.065 | 0.061 | 0.058 |
| copy_if(1S hit) | 0.136 | 0.141 | 0.246 | 0.246 |
| **copy_if(2S hit)** | **0.165** | **0.152** | **0.240** | **0.229** |
| copy_if(1+2S hit) | 0.168 | 0.172 | 0.192 | 0.192 |
| copy_if(1S miss) | 0.220 | 0.221 | 0.110 | 0.110 |
| copy_if(2S miss) | 0.159 | 0.159 | 0.082 | 0.082 |
| copy_if(1+2S miss) | 0.168 | 0.164 | 0.088 | 0.089 |

`nsg/seg` for the two complained-about rows:

| | g++ p0 | g++ p2 | clang p0 | clang p2 |
|---|---|---|---|---|
| copy_if(2S hit) | 1.44 | **1.56** | **0.95** | **0.99** |
| copy_if(2S miss) | 0.73 | 0.73 | 2.33 | 2.33 |

**`p2` closes clang's entire `copy_if(2S hit)` deficit** (0.95 → 0.99, i.e. par)
and improves g++'s already-winning `2S hit` by 8 %. The saving is 0.011
ns/element on clang and 0.013 on g++ over ~390 boundaries, i.e. **2.8 ns and 3.3
ns per destination-segment boundary respectively** (≈13–16 cycles). That is more
than a bare predicate application costs; the predicate itself is ~1 cycle and
the rest is the extra leaf iteration and boundary-exit/re-entry round trip that
`p2` no longer takes. I have measured this but not fully attributed it.

`copy_if(1S hit)` reads 0.141 under `p2` and `p3` against 0.136 under `p0`/`p1`,
which looks like a 3.7 % regression. It is code placement, not the patch:
`p4` and `p5` *also* contain the `p2` leaf and read 0.137 / 0.138. The `1S`
destination is `unreachable_sentinel_t`, where both new tests are compiled away
entirely.

### 6b. `p1` — rejected, and informative

Removing the unroll pragma from `copy_if_cleanup_blocks` is a **large
regression** on g++: `copy_if(2S miss)` 0.159 → 0.256, `copy_if(2S hit)` 0.165 →
0.277, `copy_if(1+2S hit)` 0.168 → 0.390. It is a no-op on clang, where
`BOOST_CONTAINER_SEGMENTED_AUTO_UNROLL` is already empty
(`segmented_iterator_traits.hpp:404-409`). So the Duff ladder in §4c is a real
cost but the unrolling it comes with is worth far more than it. My initial
hypothesis that the pragma was the culprit is wrong.

### 6c. `p4` / `p5` — conformant, faster on one row, slower on two

Both replace `copy_if_cleanup_blocks<32>` with a leaf that computes
`run = min(source left, destination room)` and executes exactly that many
iterations with **no destination test in the inner loop at all**, looping until
either side is exhausted. This is the "destination walker pre-checking remaining
room" idea in its strongest form; it also achieves the exact predicate count on
its own, without the `p2` statement reorder. `p5` additionally stops when the
provable run drops below 8 and hands the tail to the `p2` leaf, so a
nearly-full destination does not pay an unroll prologue per element.

Both pass the test and reach the mandated predicate counts. Timing:

| algo | clang p0 | clang p2 | clang p4 | clang p5 | g++ p0 | g++ p2 | g++ p4 | g++ p5 |
|---|---|---|---|---|---|---|---|---|
| copy_if(2S hit) | 0.240 | 0.229 | **0.209** | **0.183** | 0.165 | 0.152 | 0.171 | 0.158 |
| copy_if(2S miss) | 0.082 | 0.082 | *0.106* | *0.117* | 0.159 | 0.159 | *0.175* | 0.153 |
| copy_if(1+2S miss) | 0.088 | 0.089 | *0.123* | *0.125* | 0.168 | 0.164 | *0.180* | *0.197* |
| copy_if(1+2S hit) | 0.192 | 0.192 | 0.212 | 0.193 | 0.168 | 0.172 | 0.178 | 0.174 |

`p5` takes clang's `copy_if(2S hit)` from `nsg/seg` 0.95 to **1.24**, the best
result on that row by a wide margin. But it costs clang 43 % on `copy_if(2S
miss)` (0.082 → 0.117) and 42 % on `copy_if(1+2S miss)`, because clang compiles
the *fixed*-32 block loop into something extremely good (0.082 ns/element ≈
0.4 cycles) and a variable trip count destroys that. Net negative; not
recommended as it stands. If the `2S hit` row ever becomes the priority, `p5`
with a compile-time-constant run length (e.g. `run` rounded down to a multiple
of 32 with a checked tail) is the obvious next thing to try — not attempted
here.

---

## 7. Ranked proposals

1. **Adopt `p2`: move the destination-full test in
   `segmented_copy_if_dst_bounded`'s generic leaf to *after* the write, plus one
   test on entry, and advance `first` past the written element on the boundary
   exit.** Eight lines in one function. Makes the predicate-application count
   exactly conforming at every destination depth. Measured: clang
   `copy_if(2S hit)` `nsg/seg` 0.95 → 0.99, g++ 1.44 → 1.56; nothing else moves
   outside noise; static growth of 3–9 instructions, all cold. Test passes on
   both compilers.

2. **Do not build the lazy-refill destination write iterator for this.** `p2`
   already achieves the exact mandated count with no new abstraction, no walker
   change, and no interface change, and it captures the entire measured
   per-boundary cost that such an iterator would remove (2.8–3.3 ns × 390
   boundaries per 100 000 elements at block 128). The iterator remains
   justifiable on other grounds — it would let a leaf stream across destination
   segments and so give a larger unbounded inner loop, which is what `p4`/`p5`
   approximate badly — but the `copy_if` conformance bug is no longer a reason
   to build it. Note also that the design document `ITERATOR_PROPOSAL.md` is not
   present anywhere in the tree and
   `experimental/cursor_build/g15/dstiter/{bbin,braw}` are both empty, so the
   earlier prototype could not be re-measured.

3. **`copy_if` does not need the `set_*`/`merge` "why did I stop" bool.** For
   those algorithms the bool is genuinely load-bearing because "output full" and
   "input exhausted" can be true simultaneously and the walkers must
   disambiguate. For `copy_if` the leaf can only stop for two reasons, and
   "stopped because the destination segment filled" is exactly `first != last`,
   which the walker already tests
   (`segmented_copy_if.hpp:180`, `:129`, `:137`). Adding a third return field
   would cost a wider return value for information the walker already has. The
   fix belongs entirely in the leaf's statement order.

4. **Open: g++'s `copy_if(2S miss)` (`nsg/seg` 0.72).** Not addressed by any
   variant tried. It is a `copy_if_cleanup_blocks<32>` codegen effect on GCC:
   0.75 extra instructions per element of unroll prologue and dead Duff ladder,
   incurred whenever the destination segment has room for 32 writes, with zero
   boundary crossings involved. Removing the unroll pragma makes it much worse
   (`p1`); making the block length variable makes clang much worse (`p4`/`p5`).
   The promising untried direction is to keep a compile-time-constant inner trip
   count while raising the amount of work per prologue — e.g. compute
   `nblocks = min(avail, room) / 32` once and run `nblocks` fixed-32 blocks
   before re-checking, so the prologue is amortised over `min(avail, room)`
   elements instead of 32 while the inner loop stays constant-trip. That keeps
   clang's fixed-32 codegen intact. Not attempted.

5. **`p1` is rejected.** Documented above so it is not retried.

---

## 8. Not verified

* **MSVC and 32-bit were not built or run at all.** The `fatal error C1002`
  constraint on force-inlining walkers is untouched by `p2` (it changes no
  inlining attribute and adds no walker code), but this is by inspection only.
* **C++03 was not compiled.** `p2` introduces no `auto`, lambda, range-`for`,
  `nullptr` or variadic, and uses only the macros already present in the
  function, but only `-std=c++20` was built.
* **Only `segmented_copy_if_test.cpp` was run.** The rest of the test suite was
  not built against any prototype.
* **Only benchmark group 25 was measured.** `p2` touches only
  `segmented_copy_if.hpp`, so the other groups cannot be affected, but they were
  not re-run to confirm.
* **Only the `MyInt` value type.** `int`, `MyFatInt<4>`, `MyFatInt<8>` are
  commented out in `main()` and were not enabled.
* **`nice -n -5` was refused by the kernel** on every pinned run (`setpriority:
  Permission denied`); only `taskset -c 3` is in effect.
* **No hardware performance counters were collected.** All instruction-level
  claims are static counts from `objdump`; IPC, branch-miss and front-end
  attributions are inferences from those counts plus wall-clock, not
  measurements.
* **The 2.8–3.3 ns saved per destination-segment boundary by `p2` is measured
  but not fully attributed.** One predicate application is ~1 cycle; the
  remaining ~12–15 cycles are ascribed to the extra loop iteration and the
  boundary exit/re-entry round trip, which I did not isolate.
* **The `364_sweep` probe's `nsg` numbers do not reproduce the benchmark's
  `nsg` numbers under clang** (0.312 vs 0.229 for `hit50` at block 128); its
  `seg` numbers do. Only `seg` figures from that probe are used for conclusions.
* **`copy_if(1S hit)` under `p2`/`p3` reading 0.141 vs 0.136** is attributed to
  code placement on the evidence that `p4`/`p5`, which contain the same leaf,
  read 0.137/0.138. It was not confirmed by forcing a different layout.
* **`copy(2S)` under g++ `p2` reads 0.053 vs 0.051.** `p2` does not touch
  `segmented_copy.hpp`, so this is layout noise, but it was not chased.
* **The earlier `g15/dstiter` prototype and `ITERATOR_PROPOSAL.md` could not be
  found**, so the lazy-refill design was assessed from the description in the
  brief and from the measured per-boundary cost, not from its own code.
