# One-sided random-access leaves in dual-range segmented algorithms

Exploration only — no library code was changed.

**Question.** Current dual-range RA specializations require *both* participating iterators to be random-access. Is there still a useful optimization when only one side is RA?

**Method.** Inventory of enable conditions in the experimental headers; analysis of what one-sided RA can mathematically buy; assembly probes with g++-16 and clang++-22 (`-std=c++20 -O3`) using both thin pointer wrappers and honest linked-list forward iterators (so the compiler cannot recover `last - first` via pointer subtraction). Probe sources: `experimental/cursor_build/ra_onesided_probe.cpp`, `ra_onesided_probe2.cpp`.

---

## 1. What the library requires today

| Leaf family | Files | Enable condition | What the RA leaf does |
| --- | --- | --- | --- |
| Unconditional write (`copy`, `copy_n`, `transform`, `swap_ranges`, `reverse_copy`) | `segmented_*.hpp` | **Src RA** (tag) **and** **Dst RA** (`iterator_enable_if_tag`) | `n = min(src_n, dst_n)` then recurse into the unbounded leaf (`unreachable_sentinel_t`) |
| Conditional write (`copy_if`, `remove_copy`, `remove_copy_if`) | same | **Src RA and Dst RA** | Block-amortize: while `src_n >= B && dst_room >= B`, run a dest-check-free block of `B` |
| `partition_copy` | `segmented_partition_copy.hpp` | **Src RA and both outputs RA** | Same block idea with room on *both* outputs |
| Two-input scan (`equal` / `mismatch` via `segmented_iter2_bounded`) | `segmented_common_algo.hpp` | **Iter1 RA and Iter2 RA** | `n = min(n1, n2)` then single-exit unrolled compare loop |
| `equal` sized overload | `segmented_equal.hpp` | **Both RA** | Early `false` if `(last1-first1) != (last2-first2)` |
| `merge` / `set_*` | `segmented_merge.hpp`, `segmented_set_*.hpp` | **Iter1 RA** (tag) **and** `seg_is_ra_iterator<Iter2>` **and** `seg_is_ra_iterator<Dst>` | `merge_blocks` / `set_*_blocks`: while all three ranges have ≥ `B` room, emit a check-free block of `B` |

Common pattern: the RA leaf exists to turn a **multi-exit** loop into a **single-exit counted** loop, or to prove a **safe block size** before dropping per-element bound checks. Both need a known length (or room) on every range that can stop the loop.

---

## 2. When only one side is RA in practice

After hierarchical decomposition of `boost::container::deque`, the local iterator is `T*` — always RA. The same is true for `vector`. So for the shapes measured in the input-output article (`1S` / `2S` / `1+2S` with deque↔vector), **both leaf iterators are RA**. One-sided specializations would never be selected there.

One-sided RA appears only when the *other* argument is a genuinely non-RA iterator, for example:

- `deque` ↔ `std::list` / `forward_list`
- `deque` / `vector` ↔ filtering / transform / zip iterators that only advertise forward/bidirectional
- `deque` ↔ classic OutputIterator (`back_inserter`, stream iterators) — usually via the unbounded/`unreachable_sentinel_t` path rather than a bounded segment

So the question is about **interop with non-RA ranges**, not about the deque↔deque / deque↔vector hot path.

---

## 3. Candidate one-sided transforms

### 3.1 Unconditional `copy`-family, bounded destination

| Side that is RA | Possible rewrite | Restores single-exit? | Enables SIMD? |
| --- | --- | --- | --- |
| **Both** (current) | `n = min(src,dst)`; unbounded leaf | Yes | Yes (GCC & Clang) |
| **Dst only** | `n = dst_last - dst_first`; `for (; n && first != last; --n, …)` | **No** (still source-end + count) | **No** with real forward src |
| **Src only** | `n = last - first`; `for (; n && dst != dst_last; --n, …)` | **No** | **No** with real forward dst |
| **Src + unbounded dest** (sentinel) | `for (; first != last; …)` or counted `n = last-first` | Yes (dest check folds away) | Yes when `++dst` is a pointer bump — **already true of the generic leaf** today |

### 3.2 Conditional `copy_if`-family

| Side that is RA | Possible rewrite | Notes |
| --- | --- | --- |
| **Both** (current) | Block while `src_n >= B && dst_room >= B` | Fixed trip count of `B` with no dest check and no src-end check inside the block |
| **Dst only** | Block while `dst_room >= B`, but each element must still test `first != last` | Cannot prove “exactly B source elements remain”. Assembly shows *more* per-element compares after unroll (see §4). Unlikely win. |
| **Src only + unbounded dest** | Counted scan, no dest check | Already available via sentinel fold; RA-src length is optional sugar |

### 3.3 `equal` / `mismatch` (`segmented_iter2_bounded`)

| Side that is RA | Possible rewrite | Notes |
| --- | --- | --- |
| **Both** | `n = min`; single-exit compare loop | Removes one bound check from the hot path; early exit on mismatch still prevents SIMD in these probes |
| **One side** | Countdown on the RA side + `!=` on the other | Still two exit conditions — essentially the generic shape |
| **Both** (sized `equal`) | O(1) length inequality → `false` | **Impossible** with only one RA side (other length is O(N)) |

### 3.4 `merge` / `set_*` / `partition_copy`

The block helpers call `seg_srcs_dst_room_at_least`, which needs **O(1) remaining length on every range that the block will consume without re-checking**. If any source is only forward:

```
while (dst_room >= B && a != aend && b != bend) {
   for (B times) { /* pick from a or b; ++ */ }  // WRONG: a or b may end mid-block
}
```

Fixing that reintroduces per-element source-end tests and destroys the point of the block. **One-sided RA is insufficient for the merge/set/partition_copy block optimization.** Partial variants (e.g. both sources RA, dest not) are conceivable for unbounded destinations (`unreachable_sentinel_t` already drops the dest length check in `seg_srcs_dst_room_at_least`), but that is “dest unbounded”, not “dest non-RA bounded”.

---

## 4. Assembly evidence (g++-16 / clang++-22, `-O3`)

Probes used an honest `ListIt` over `Node{value, next}` so distance is not recoverable by pointer arithmetic. Summary (`simd` = presence of `movdqu`/`movups`/etc.):

| Leaf shape | GCC SIMD | Clang SIMD |
| --- | --- | --- |
| `copy` both pointers, two-exit generic | no | **yes** (Clang recovers `min` itself) |
| `copy` both-RA `min` then single-exit | **yes** | **yes** |
| `copy` RA dst + list src (countdown) | no | no |
| `copy` generic list src + RA dst | no | no |
| `copy` RA src + list dst (countdown) | no | no |
| `copy` RA src + unbounded pointer dst | **yes** | **yes** |
| `copy_if` both-RA blocked | no (unrolled scalar) | no (unrolled scalar) |
| `copy_if` RA dst + list src “block” | no (heavier) | no |
| `equal` both-RA `min` | no | no |
| `equal` RA + list (either side) | no | no |

### 4.1 GCC: both-RA is the SIMD switch for bounded `copy`

Generic two-exit (scalar):

```asm
.L8:
        movl    (%rdi), %eax
        addq    $4, %rdi
        addq    $4, %rdx
        movl    %eax, -4(%rdx)
        cmpq    %rdi, %rsi          ; src end
        je      .L4
.L3:
        cmpq    %rdx, %rcx          ; dst end
        jne     .L8
```

Both-RA (vectorized):

```asm
        ; n = min(src_n, dst_n) via sub/sar/cmovl ...
.L14:
        movdqu  (%rdi,%rdx), %xmm0
        movups  %xmm0, (%rax,%rdx)
        addq    $16, %rdx
        cmpq    %rdx, %rsi
        jne     .L14
```

RA-dst + list src (countdown, still scalar, still two exits — note `movq 8(%rdi), %rdi` list walk):

```asm
.L30:
        movl    (%rdi), %edx
        addq    $4, %rax
        movq    8(%rdi), %rdi       ; ++list
        movl    %edx, -4(%rax)
        subq    $1, %rcx            ; --dst_room
        je      .L29
.L26:
        cmpq    %rdi, %rsi          ; list end
        jne     .L30
```

One-sided countdown is **not** meaningfully different from the generic list leaf: same two-exit scalar structure, same dependent list load. No vectorization.

### 4.2 Clang nuance

On **two raw pointers**, Clang’s vectorizer already rewrites the generic two-exit `copy` into a `min`-then-SIMD loop. So for pointer/pointer pairs the both-RA specialization is partly redundant on Clang (still useful as a portable, explicit contract and for GCC).

On **list ↔ pointer**, Clang does **not** vectorize. One-sided RA does not create a new SIMD opportunity Clang was missing.

### 4.3 `copy_if` block with only dest RA

The both-RA block can omit *both* the destination check and the source-end check inside the `B` body (source length was pre-checked). With only dest RA, every element still needs `first != last`. After GCC unrolls that “up-to-B” loop, the probe shows ~23 `cmpq` vs ~4 in the simple generic leaf — more code, same asymptotic work, no SIMD. That is a likely **pessimization**, not a win.

### 4.4 `equal` / `mismatch`

Neither compiler SIMD-vectorized the compare loops (early-exit dependence). Both-RA still helps by dropping one bound check and enabling the sized-`equal` O(1) length reject. One-sided RA keeps two exits and cannot implement the sized length check.

---

## 5. Mapping back to algorithm families

| Algorithm family | One-sided RA worth adding? | Why |
| --- | --- | --- |
| `copy` / `copy_n` / `transform` / `swap_ranges` / `reverse_copy` (bounded) | **No meaningful win** | Need `min` of both lengths for single-exit SIMD. One-sided countdown stays two-exit and scalar for real forward iterators. |
| Same, unbounded destination | **Already covered** | `unreachable_sentinel_t` folds the dest check; RA src is enough for GCC/Clang to vectorize when `++dst` is cheap. No new specialization required. |
| `copy_if` / `remove_copy[_if]` | **No** | Block amortization needs a known source run length to drop src-end checks; dest-only blocks add compares. |
| `partition_copy` | **No** | Needs src + both outputs for safe blocks. |
| `equal` / `mismatch` leaf | **Marginal at best** | One-sided countdown ≈ generic two-exit; both-RA’s real extras are single-exit + sized length check (needs both). |
| `merge` / `set_*` | **No** (for non-RA source) | Block room proof requires all consumed ranges to be RA (or unbounded dest). |

---

## 6. Conclusions

1. **The both-RA requirement is load-bearing, not accidental.** The profitable transforms are (a) `n = min(n1,n2)` → single-exit loop and (b) “all ranges have ≥ B remaining” → check-free block. Both need an O(1) length/room on every range that can stop the leaf.

2. **One-sided RA does not restore those transforms** when the non-RA side is a true forward/bidirectional iterator (list, filtered views, etc.). Assembly confirms: no SIMD on GCC or Clang; loops remain two-exit scalar with list-node chasing.

3. **The important one-sided case is already handled:** RA (or pointer) source + unbounded destination via `unreachable_sentinel_t`. That is the `1S` (segmented input, flat output) path. Adding an explicit “RA src only” overload would be redundant for that shape.

4. **Clang can synthesize `min` from two pointers even without the RA leaf; GCC cannot.** Portable speedups for bounded dual-RA leaves still justify the current both-RA specializations (as the Group-25 RA0 vs RA1 benchmarks showed on GCC). That Clang trick does **not** extend to one-sided list/forward cases.

5. **For Boost.Container’s primary segmented containers (`deque` locals = `T*`), dual-range leaves almost always see RA on both sides.** One-sided specializations would mainly target exotic interop (`deque`↔`list`) where the list walk dominates anyway — a poor ROI relative to the both-RA work already done.

6. **Recommendation:** keep requiring both (or all) sides RA for dual-range leaf specializations. Do not add one-sided RA overloads unless a concrete, measured interop workload appears; if it does, the only plausible micro-win to re-check is a dest-capacity countdown vs `dst == dst_last` on platforms where that compare is expensive — and even then, expect scalar code and small effects, not the SIMD-class wins of the both-RA path.

---

## Annex: probe artifacts

| File | Role |
| --- | --- |
| `experimental/cursor_build/ra_onesided_probe.cpp` | First probe (thin `FwdInt` wrapper — Clang saw through pointer equality) |
| `experimental/cursor_build/ra_onesided_probe2.cpp` | Honest `ListIt` / `Node` probe used for conclusions |
| `ra_onesided_probe2.gcc.s` / `.clang.s` | Generated assembly |
| `summarize_asm2.py` / `dump_key_asm.py` | SIMD / snippet helpers |
| `ra_onesided_probe2.key_asm.txt` | Extracted key functions |
