# Neoclassical C++ (2): Exploring input-output segmented algorithms

## From one segmented range to two

In the [first installment](https://boostedcpp.net/2026/05/18/neoclassical-c-segmented-iterators-revisited-1/) of this series we revisited Matt Austern's *Segmented Iterators and Hierarchical Algorithms* paper and measured what a modern compiler can do when a single-range, single-pass algorithm (`find`, `fill`, `count`, `for_each`, ...) is decomposed into per-segment flat loops. The results were encouraging: once the per-element block-boundary check disappears from the inner loop, auto-vectorisers wake up and speedups of 3×-6× (with 17× corner cases) appear on small trivially-comparable types.

This second installment moves one step up in difficulty: algorithms that read from one range and *write* to another one, such as `copy`, `copy_n`, `copy_if`, `remove_copy`, `remove_copy_if`, `swap_ranges` and `transform`. Now there are two ranges in play and *each* of them may be segmented, may be flat, or both at the same time.

The good news is that the two ranges are independent: the input walks forward, the output walks forward, and neither influences the shape of the other. The bad news is that the output side is not symmetrical to the input side, and this asymmetry is where all the interesting problems (and all the interesting performance results) of this article come from.

## Challenges with the output range

### Iterating the output to obtain segment bounds

Austern's original scheme decomposes a *range* `[first, last)` into its segments: **you need both endpoints (first + last) to discover the (potentially recursive) segment boundaries**. His paper's worked example is `hierarchical_fill`, and its shape is the template that every single-range hierarchical algorithm follows:

```cpp
hierarchical_fill(first, last, value):
    sfirst = segment(first)            // segment containing the first element
    slast  = segment(last)             // segment containing the one-past-last

    if sfirst == slast: // whole range lives in one segment
        fill_dispatch(local(first), local(last), value)
    else:
        fill_dispatch(local(first), end(sfirst), value)	//first segment: partial

        for seg = sfirst + 1 while seg != slast, ++seg: // middle segments: full
            fill_dispatch(begin(seg), end(seg), value)

        fill_dispatch(begin(slast), local(last), value)	// last segment: partial
```

The three `fill_dispatch` calls need a `local_iterator` **range**; if that `local_iterator` is itself segmented, `fill_dispatch` calls `hierarchical_fill` again until the local iterator is flat, at which point it calls the classic STL-like `fill`. However, the STL **output interface provides only a single output iterator** (`std::copy(first, last, result)`) with no `result_end` argument and thus no `segment(result_end)` to compute and follow Austern's decomposition. `result` is assumed **unbounded**.

### State of the art for segmented outputs

This is less charted than the input segmentation side, but there are some interesting implementations. Some examples:

- **libstdc++** specializes output iterator segmentation for `deque` in [`bits/deque.tcc`](https://github.com/gcc-mirror/gcc/blob/master/libstdc%2B%2B-v3/include/bits/deque.tcc). Its destination overload of `__copy_move_a1` walks deque blocks, but it needs a random-access input and, since it targets `deque` specifically, it is neither recursive nor extensible.

- **libc++** is the closest prior art, with all the machinery private (not intended for user iterators). [`__segmented_iterator_traits`](https://github.com/llvm/llvm-project/blob/main/libcxx/include/__iterator/segmented_iterator.h) implements Austern's protocol (`__segment`/`__local`/`__begin`/`__end`/`__compose`), and [`copy.h`](https://github.com/llvm/llvm-project/blob/main/libcxx/include/__algorithm/copy.h) has a segmented-destination overload handling random-access local iterators.

- **HPX** offers a *public* traits framework ([`segmented_iterator_traits`](https://github.com/TheHPXProject/hpx/blob/master/libs/core/algorithms/include/hpx/algorithms/traits/segmented_iterator_traits.hpp)). The [`segmented_copy`](https://github.com/STEllAR-GROUP/hpx/blob/master/libs/full/segmented_algorithms/include/hpx/parallel/segmented_algorithms/copy.hpp) algorithm uses local iterators for both input and output and recomposes both sides, but advances source and destination segments in lockstep, so it does not handle mismatched segment boundaries.

This article will try to outline a scheme that is simultaneously **generic**, **recursive** and supports **independent segmentation on both sides**, together with the problems encountered and the benchmark results.

### Handling the missing bound for output iterators

The proposal is a two-phase approach that handles the unbounded output iterator and discovers (recursively) its segment boundaries.
- **Phase 1: Unbounded, outermost level**: Like some of the mentioned libraries do, we ask `segmented_iterator_traits` for the segment it belongs to (`result_seg = segment(result)`) and obtain the end of that segment (`local_result_end = end(result_seg)`). We don't know where this _"top"_ segment concatenation will end (as `result` is unbounded), but we do know that there should be enough segments to hold the number of elements to be written. When this top-level segment (`[local(result), local_result_end)`) is processed, we advance to the next segment (`++result_seg`) and continue processing. **This level is enough to handle single-level segmented types like `deque`**.
- **Phase 2: Bounded, inner levels**: Once we have a bounded range `[local(result), local_result_end)`, then we can use Austern's pattern. The inner `local_iterator` lives inside a segment whose extent was chosen by its parent, so it is a *bounded* range: when it runs out of room it has to stop and report back, so that the parent can advance *its* segment iterator and process a fresh sub-range. **These levels are needed for multi-level segmentations like multi-dimensional matrix types**.

<img src="https://boostedcpp.net/wp-content/uploads/2026/08/two_phase_segmented_output.png" alt="Two-phase segmented output" class="aligncenter size-full" />

So let's try to write some pseudo-code implementing this idea for output iterators. For simplicity, let's ignore any input range segmentation:

```cpp
unbounded_copy(first, last, result):        // result is unbounded
    result_seg   = segment(result)          // Obtain result's current segment
    local_result = local(result)            // Obtain result's local position

    while first != last:                    // Unbounded output loop
       local_result_end = end(result_seg)   // Obtain the missing local bound

       (first, local_result) =              // [local_result, local_result_end) is *bounded*
          bounded_copy(first, last, local_result, local_result_end)

       if first != last:                    // if destination segment full
          ++result_seg                      // only outermost may advance
          local_result = begin(result_seg)

    return compose(result_seg, local_result)

bounded_copy(first, last, dst, dst_end):    // both ends known
    if dst is segmented:
       // Austern first/middle/last over [dst, dst_end) calling bounded_copy recursively
    else:
       // leaf: process until input or destination exhausted
       while first != last and dst != dst_end:
          *dst++ = *first++
       return (first, dst)
```

## Writing `hierarchical_copy_if` supporting input + output segmented iterators

Before explaining the difficulties and the code, it is worth recalling what we are starting from. This is the canonical implementation of `std::copy_if`, essentially unchanged since the original HP STL:

```cpp
template <class InputIt, class OutputIt, class Pred>
OutputIt copy_if(InputIt first, InputIt last, OutputIt result, Pred pred)
{
   for (; first != last; ++first)
      if(pred(*first)) {
         *result = *first;
         ++result;
      }
   return result;
}
```

It's incredibly simple code, with no helper functions and no traits.

### Main steps to write `hierarchical_copy_if` in classic C++

In order to write a `hierarchical_copy_if` that is generic, recursive and supports independent segmentation on both sides, we should:

- Implement the Austern pattern handling a potentially recursively segmented input range.
- Once the input range has been peeled down to the flat (non-segmented) level, dispatch the implementation depending on the output iterator segmentation.
- Implement the unbounded segmentation phase for the output iterator.
- Implement the bounded segmentation phase for the output iterator.
- Implement the leaf algorithm where both input and output iterators are flat and bounded. This leaf must return information about both iterators so that the parent can handle segmentation.

### Note 1: Effect of bounded destinations

In the classic STL interface the loop of the `copy`-family algorithms is bounded only by the *input*. That's an advantage for the code generator, even if the output iterator is complex (e.g. `deque::iterator`), as the innermost loop has exactly one termination condition:

```cpp
for(; first != last; ++first) {
   *dst_first = *first;
   ++dst_first;
}
```

The produced code is efficient, and if the branch predictor does a good job for the `deque::iterator` internal segment logic, the CPU can process that loop quite efficiently. But once the destination is segmented, the write loop must stop at whichever comes first: input exhausted *or* destination segment full:

```cpp
for(; first != last; ++first) {
   if(dst_first == dst_last)   //destination segment full?
      break;
   *dst_first = *first;
   ++dst_first;
}
```

This implies two loop-carried exit conditions instead of one, and this shape is not something that auto-vectorisers like; we will see the effect clearly in the benchmarks, especially when additional conditions (like predicates) are handled inside the leaf loop.

### Note 2: Predicate call contracts

The second complication is related to correctness. For the conditionally-writing algorithms (`copy_if`, `remove_copy`, `remove_copy_if`) the standard mandates exactly *last - first* applications of the predicate. A hierarchical implementation, once both the input and output ranges have been flattened, splits the work into many leaf calls, and a leaf call may stop *in the middle of the input* because its destination segment filled up.

The obvious way to write the bounded leaf is to test the destination just before writing:

```cpp
for(; first != last; ++first) {
   if(pred(*first)) {
      if(dst_first == dst_last)   //destination segment full?
         break;                   //Predicate already called but output not produced
      *dst_first = *first;
      ++dst_first;
   }
}
```

But this approach is correct only at a price, as the loop can stop *between* testing an element and writing it. If the upper layer advances to the next destination segment and calls the leaf again, it resumes on that same element and `pred` is applied to it twice, breaking the guarantee. To avoid it, the leaf would have to report a third piece of state (_"a write is owed on `*first`"_) and **every enclosing level would need a special case to settle that write** before resuming and calling the predicate again.

Another alternative is to *reorder* the test: check the destination once on entry, and then again only after a successful write:

```cpp
if(dst_first == dst_last)         //no room at all: nothing tested yet
   return {first, dst_first};

for(; first != last; ++first) {
   if(pred(*first)) {
      *dst_first = *first;
      ++dst_first;
      if(dst_first == dst_last) { //destination segment full?
         ++first;                 //consume the element just written
         break;
      }
   }
}
return {first, dst_first};
```

Now the leaf never stops in the gap. On return, `first` either equals `last` or points at an element the predicate has *never seen*, so the caller simply calls again with a fresh destination segment. In any case, the predicate guarantee implies extra per-segment tests that can hurt performance, as those checks might subtly affect inlining, instruction cache usage and branch prediction.

### `hierarchical_copy_if`, a possible implementation

Now, let's take a deep breath. Implementing all the steps and notes outlined before is no longer _simple_:

```cpp
// (1) LEAF — both iterators are flat. Stops on input exhausted or destination full,
// and reports both final positions.  To properly implement the predicate call guarantee,
// on return 'first' is always at an element pred has never seen, so the
// caller can resume with a fresh segment and no extra state.
template <class InIt, class DstIt, class Pred>
pair<InIt, DstIt> copy_if_bounded
   (InIt first, InIt last, DstIt dst, DstIt dst_end, Pred pred, non_segmented_iterator_tag)
{
   if (dst == dst_end)                 // no room: nothing tested
      return {first, dst};

   for (; first != last; ++first) {
      if (pred(*first)) {              // exactly once per element, ever
         *dst = *first;
         ++dst;
         if (dst == dst_end) {
            ++first;                   // consume the element just written
            break;
         }
      }
   }
   return {first, dst};
}

// (2) BOUNDED SEGMENTED DESTINATION — used at every level below the outermost.
// Both destination endpoints are known, so Austern's decomposition is applied.
template <class InIt, class SegDst, class Pred>
pair<InIt, SegDst> copy_if_bounded
   (InIt first, InIt last, SegDst dst_first, SegDst dst_last, Pred pred, segmented_iterator_tag)
{
   using out_tr    = segmented_iterator_traits<SegDst>;
   using local_it  = typename out_tr::local_iterator;
   using seg_it    = typename out_tr::segment_iterator;
   using local_tag = typename segmented_iterator_traits<local_it>::is_segmented_iterator;

   seg_it       sfirst = out_tr::segment(dst_first);
   const seg_it slast  = out_tr::segment(dst_last);

   if (sfirst == slast) {   //whole output range in one segment
      auto r = copy_if_bounded
         (first, last, out_tr::local(dst_first), out_tr::local(dst_last), pred, local_tag());
      return {r.first, out_tr::compose(sfirst, r.second)};
   }

   auto r = copy_if_bounded // first: [local(dst_first), end(sfirst))
      (first, last, out_tr::local(dst_first), out_tr::end(sfirst), pred, local_tag());
   first = r.first;
   if (first == last)
      return {first, out_tr::compose(sfirst, r.second)};

   for (++sfirst; sfirst != slast; ++sfirst) {  // middle: [begin(seg), end(seg))
      r = copy_if_bounded
         (first, last, out_tr::begin(sfirst), out_tr::end(sfirst), pred, local_tag());
      first = r.first;
      if (first == last)
         return {first, out_tr::compose(sfirst, r.second)};
   }

   r = copy_if_bounded  // last: [begin(slast), local(dst_last))
      (first, last, out_tr::begin(sfirst), out_tr::local(dst_last), pred, local_tag());
   return {r.first, out_tr::compose(sfirst, r.second)};
}

// (3) UNBOUNDED DESTINATION, SEGMENTED — the outermost segmentation level for destination
template <class InIt, class SegOut, class Pred>
SegOut copy_if_dst_dispatch(InIt first, InIt last, SegOut result, Pred pred, segmented_iterator_tag)
{
   using out_tr    = segmented_iterator_traits<SegOut>;
   using local_it  = typename out_tr::local_iterator;
   using seg_it    = typename out_tr::segment_iterator;
   using local_tag = typename segmented_iterator_traits<local_it>::is_segmented_iterator;

   if (first == last)
      return result;

   seg_it   seg   = out_tr::segment(result);
   local_it local = out_tr::local(result);

   for (;;) { // Convert unbounded iterator into a bounded local iterator range.
      auto r = copy_if_bounded(first, last, local, out_tr::end(seg), pred, local_tag());
      first = r.first;
      if (first == last)
         return out_tr::compose(seg, r.second);
      ++seg;        // go to the next segment
      local = out_tr::begin(seg);
   }
}

// (3b) the classic STL case: no output bound, so no destination test at all.
template <class InIt, class OutIt, class Pred>
OutIt copy_if_dst_dispatch(InIt first, InIt last, OutIt result, Pred pred, non_segmented_iterator_tag)
{
   for (; first != last; ++first) {
      if (pred(*first)) {
         *result = *first;
         ++result;
      }
   }
   return result;
}

// (4) SOURCE SIDE — Austern first/middle/last over the input.
template <class SegIn, class OutIt, class Pred>
OutIt copy_if_src_dispatch(SegIn first, SegIn last, OutIt result, Pred pred, segmented_iterator_tag)
{
   using tr    = segmented_iterator_traits<SegIn>;
   using local_it  = typename tr::local_iterator;
   using seg_it    = typename tr::segment_iterator;
   using local_tag = typename segmented_iterator_traits<local_it>::is_segmented_iterator;

   seg_it       sfirst = tr::segment(first);
   const seg_it slast  = tr::segment(last);

   if (sfirst == slast)
      return copy_if_src_dispatch(tr::local(first), tr::local(last), result, pred, local_tag());

   result = copy_if_src_dispatch(tr::local(first), tr::end(sfirst), result, pred, local_tag());
   for (++sfirst; sfirst != slast; ++sfirst)
      result = copy_if_src_dispatch(tr::begin(sfirst), tr::end(sfirst), result, pred, local_tag());
   return copy_if_src_dispatch(tr::begin(sfirst), tr::local(last), result, pred, local_tag());
}

// (4b) Source flat: hand over to the destination side.
template <class InIt, class OutIt, class Pred>
OutIt copy_if_src_dispatch(InIt first, InIt last, OutIt result, Pred pred, non_segmented_iterator_tag)
{
   using out_traits = segmented_iterator_traits<OutIt>;
   return copy_if_dst_dispatch
      (first, last, result, pred, typename out_traits::is_segmented_iterator());
}

// (5) MAIN ALGORITHM — dispatch on input segmentation;  destination is handled later.
template <class InIt, class OutIt, class Pred>
OutIt hierarchical_copy_if(InIt first, InIt last, OutIt result, Pred pred)
{
   using in_tr = segmented_iterator_traits<InIt>;
   return copy_if_src_dispatch
      (first, last, result, pred, typename in_tr::is_segmented_iterator());
}
```

## How Boost.Container is experimenting with output segmentation

[Boost.Container's](https://www.boost.org/doc/libs/latest/doc/html/container.html) experimental headers under `boost/container/experimental/` turn the design sketched above into a working prototype library. The goal is not a drop-in replacement for `<algorithm>` yet — it is a testbed for a traits-driven, recursively segmented algorithm suite that can be measured against `std::` on the same containers and iteratively refined. The main design points:

**Extensible traits.** [`segmented_iterator_traits`](https://github.com/boostorg/container/blob/develop/include/boost/container/experimental/segmented_iterator_traits.hpp) exposes Austern's protocol — `segment` / `local` / `begin` / `end` / `compose` — as a public customization point. User iterators can opt in by specializing the traits.

**Recursive on both sides.** Every dispatch peels one level and asks whether the `local_iterator` is itself segmented. The output iterator segmentation is handled using the explained **two-phase output.**

**Independent segmentation, not lockstep.** Source and destination are walked by separate outer loops. A source segment may spill across several destination segments, and vice versa.

**Predicate-once contracts.** Conditionally writing algorithms (`segmented_copy_if`, `segmented_remove_copy_if`) honour the classic STL rule of exactly `last - first` predicate applications.

So in the next chapter we'll use that experimental Boost code to benchmark segmented input-output algorithms and compare them with the Standard library.

## Benchmarking segmented input-output algorithms

The invariants of the benchmarks are essentially the same as in the first article:

- Every test runs against `boost::container::deque` with a fixed block size of 128 elements per block, with `size() == 100'000`.
- The per-call cost is averaged over several thousand invocations.
- The container is filled with contiguous positive values `0, 1, 2, …, N-1` (all elements non-negative, distinct, strictly ascending).
- Where the algorithm takes a predicate or a value, the harness probes both a `(hit)` and a `(miss)` variant.

Since these are two-range algorithms, each of them is measured in three *shape* variants that describe which of the two ranges is segmented:

| Shape | Input range | Output range |
| --- | --- | --- |
| `1S` | segmented (`deque`) | flat (`vector`) |
| `2S` | flat (`vector`) | segmented (`deque`) |
| `1+2S` | segmented (`deque`) | segmented (`deque`) |

The seven algorithms and their hit/miss variants (30 sub-benchmarks in total, each in the three shapes above):

| Algorithm | Hit case | Miss case |
| --- | --- | --- |
| `copy` | Copies all `N` elements. | — |
| `copy_n` | Copies the first `N` elements through the `_n` overload. | — |
| `copy_if` | Predicate `is_odd(x)` — true for half the elements; half the writes happen. | Predicate `x < 0` — never true; full scan, zero writes. |
| `remove_copy` | Value = `N/2` — present exactly once; all but one element written. | Value = `-1` — never present; all elements written. |
| `remove_copy_if` | Predicate `x < N/4 or x > 3N/4` — true for half the elements; half the writes happen. | Predicate `x < 0` — never true; all elements written. |
| `swap_ranges` | Swaps all `N` elements between the two ranges. | — |
| `transform` | Applies `x + 1` to every element, writing the result. | — |

Three element types are used this time:

- `MyInt` — wraps a single `int` (4 bytes, trivially-comparable).
- `MyFatInt<4>` — wraps four `int`s (16 bytes), comparing only its first field.
- `MyFatInt<8>` — wraps eight `int`s (32 bytes), comparing only its first field.

_Note: all three types have user-provided copy operations, so they are *not* trivially copyable. Neither the standard library nor Boost can collapse any of these copies into `memmove`; every measurement below reflects genuine loop code generation_.

As in the first article, each algorithm is executed in three modes, and for every algorithm and every value type the benchmark prints three columns:

| Column | What it isolates | Meaning of ratio > 1.0 |
| --- | --- | --- |
| `nsg/seg` | Same Boost implementation, same iterator object, with segmentation advertised or not | Boost segmented algorithm path is X times faster than the Boost non-segmented path |
| `std/seg` | How the segmented version compares to the platform's stock implementation | Boost segmented path is X times faster than the `std::` algorithm |
| `std/nsg` | How the platform's stock implementation compares to the same flat loop without the segmentation tag | Boost non-segmented path is X times faster than the `std::` algorithm, this ratio should be near 1.0 |

Tested compilers / standard libraries:

- MSVC 2026 x64 — Toolset v14.51, Microsoft STL — Windows, `/O2 /std:c++20`.
- GCC 16 — WSL Ubuntu 26.04, libstdc++, `-O3 -std=c++20`.
- Clang 22 — WSL Ubuntu 26.04, libstdc++, `-O3 -std=c++20`.

## First benchmark: Segmented input-output algorithms

The first configuration is the direct generalisation of the first article: the input range is decomposed into segments exactly as before, the output range is decomposed with the same (potentially recursive) technique, and the leaves are bounded write loops carrying the two exit conditions shown earlier, plus the bookkeeping needed to honour the predicate-call contract.

Geomean per compiler, split by shape:

#### Shape `1S` — 1S (segmented input, flat output)

T = `MyInt`:

| Compiler | `nsg/seg` | `std/seg` | `std/nsg` |
| --- | --- | --- | --- |
| GCC 16 | 1.55 | 1.61 | 1.04 |
| Clang 22 | 3.07 | 2.97 | 0.97 |
| MSVC 2026 | 5.12 | 5.15 | 1.00 |

T = `MyFatInt<4>`:

| Compiler | `nsg/seg` | `std/seg` | `std/nsg` |
| --- | --- | --- | --- |
| GCC 16 | 0.96 | 0.97 | 1.01 |
| Clang 22 | 1.38 | 1.23 | 0.90 |
| MSVC 2026 | 2.82 | 2.80 | 0.99 |

T = `MyFatInt<8>`:

| Compiler | `nsg/seg` | `std/seg` | `std/nsg` |
| --- | --- | --- | --- |
| GCC 16 | 1.03 | 1.03 | 1.00 |
| Clang 22 | 1.06 | 1.03 | 0.96 |
| MSVC 2026 | 1.81 | 1.75 | 0.97 |

Per-algorithm for `MyInt`, shape `1S` (geomean of the three compilers; per-compiler breakdowns are in the Annex):

| Algorithm | `nsg/seg` | `std/seg` | `std/nsg` |
| --- | --- | --- | --- |
| `copy` | 4.56 | 4.70 | 1.03 |
| `copy_if(hit)` | 1.90 | 1.88 | 0.99 |
| `copy_if(miss)` | 2.94 | 2.24 | 0.76 |
| `copy_n` | 2.92 | 2.84 | 0.97 |
| `remove_copy(hit)` | 2.32 | 2.74 | 1.18 |
| `remove_copy(miss)` | 2.28 | 2.85 | 1.25 |
| `remove_copy_if(hit)` | 1.88 | 1.85 | 0.98 |
| `remove_copy_if(miss)` | 2.31 | 2.31 | 1.00 |
| `swap_ranges` | 4.64 | 4.47 | 0.96 |
| `transform` | 5.32 | 5.14 | 0.97 |
| **geomean** | **2.90** | **2.91** | **1.00** |

<img src="https://boostedcpp.net/wp-content/uploads/2026/08/bench1_per_algo_myint_1s.png" alt="First benchmark: std/seg per algorithm, T = MyInt, shape = 1S" class="aligncenter size-full" />

<img src="https://boostedcpp.net/wp-content/uploads/2026/08/bench1_per_algo_myfatint4_1s.png" alt="First benchmark: std/seg per algorithm, T = MyFatInt&lt;4&gt;, shape = 1S" class="aligncenter size-full" />

<img src="https://boostedcpp.net/wp-content/uploads/2026/08/bench1_per_algo_myfatint8_1s.png" alt="First benchmark: std/seg per algorithm, T = MyFatInt&lt;8&gt;, shape = 1S" class="aligncenter size-full" />

#### Shape `2S` — 2S (flat input, segmented output)

T = `MyInt`:

| Compiler | `nsg/seg` | `std/seg` | `std/nsg` |
| --- | --- | --- | --- |
| GCC 16 | 1.05 | 1.02 | 0.98 |
| Clang 22 | 0.77 | 0.83 | 1.08 |
| MSVC 2026 | 1.86 | 1.88 | 1.01 |

T = `MyFatInt<4>`:

| Compiler | `nsg/seg` | `std/seg` | `std/nsg` |
| --- | --- | --- | --- |
| GCC 16 | 1.04 | 1.04 | 1.00 |
| Clang 22 | 1.03 | 1.04 | 1.01 |
| MSVC 2026 | 1.42 | 1.35 | 0.96 |

T = `MyFatInt<8>`:

| Compiler | `nsg/seg` | `std/seg` | `std/nsg` |
| --- | --- | --- | --- |
| GCC 16 | 1.02 | 1.02 | 1.00 |
| Clang 22 | 1.01 | 1.02 | 1.00 |
| MSVC 2026 | 1.23 | 1.19 | 0.97 |

Per-algorithm for `MyInt`, shape `2S` (geomean of the three compilers; per-compiler breakdowns are in the Annex):

| Algorithm | `nsg/seg` | `std/seg` | `std/nsg` |
| --- | --- | --- | --- |
| `copy` | 1.63 | 1.64 | 1.00 |
| `copy_if(hit)` | 0.93 | 0.92 | 0.99 |
| `copy_if(miss)` | 0.77 | 0.68 | 0.87 |
| `copy_n` | 1.43 | 1.59 | 1.12 |
| `remove_copy(hit)` | 0.96 | 0.95 | 1.00 |
| `remove_copy(miss)` | 1.02 | 1.01 | 0.99 |
| `remove_copy_if(hit)` | 0.97 | 1.00 | 1.03 |
| `remove_copy_if(miss)` | 1.04 | 1.28 | 1.23 |
| `swap_ranges` | 1.49 | 1.60 | 1.07 |
| `transform` | 1.56 | 1.47 | 0.94 |
| **geomean** | **1.14** | **1.17** | **1.02** |

<img src="https://boostedcpp.net/wp-content/uploads/2026/08/bench1_per_algo_myint_2s.png" alt="First benchmark: std/seg per algorithm, T = MyInt, shape = 2S" class="aligncenter size-full" />

<img src="https://boostedcpp.net/wp-content/uploads/2026/08/bench1_per_algo_myfatint4_2s.png" alt="First benchmark: std/seg per algorithm, T = MyFatInt&lt;4&gt;, shape = 2S" class="aligncenter size-full" />

<img src="https://boostedcpp.net/wp-content/uploads/2026/08/bench1_per_algo_myfatint8_2s.png" alt="First benchmark: std/seg per algorithm, T = MyFatInt&lt;8&gt;, shape = 2S" class="aligncenter size-full" />

#### Shape `1+2S` — 1+2S (segmented input and output)

T = `MyInt`:

| Compiler | `nsg/seg` | `std/seg` | `std/nsg` |
| --- | --- | --- | --- |
| GCC 16 | 1.36 | 1.36 | 1.00 |
| Clang 22 | 1.97 | 2.04 | 1.03 |
| MSVC 2026 | 3.76 | 3.64 | 0.97 |

T = `MyFatInt<4>`:

| Compiler | `nsg/seg` | `std/seg` | `std/nsg` |
| --- | --- | --- | --- |
| GCC 16 | 1.30 | 1.36 | 1.04 |
| Clang 22 | 1.27 | 1.25 | 0.99 |
| MSVC 2026 | 2.44 | 1.94 | 0.79 |

T = `MyFatInt<8>`:

| Compiler | `nsg/seg` | `std/seg` | `std/nsg` |
| --- | --- | --- | --- |
| GCC 16 | 1.08 | 1.09 | 1.01 |
| Clang 22 | 1.16 | 1.15 | 0.99 |
| MSVC 2026 | 1.75 | 1.25 | 0.71 |

Per-algorithm for `MyInt`, shape `1+2S` (geomean of the three compilers; per-compiler breakdowns are in the Annex):

| Algorithm | `nsg/seg` | `std/seg` | `std/nsg` |
| --- | --- | --- | --- |
| `copy` | 3.54 | 3.54 | 1.00 |
| `copy_if(hit)` | 2.09 | 1.99 | 0.95 |
| `copy_if(miss)` | 2.07 | 2.01 | 0.97 |
| `copy_n` | 3.42 | 3.69 | 1.08 |
| `remove_copy(hit)` | 1.54 | 1.52 | 0.98 |
| `remove_copy(miss)` | 1.93 | 1.89 | 0.98 |
| `remove_copy_if(hit)` | 1.43 | 1.46 | 1.02 |
| `remove_copy_if(miss)` | 1.90 | 1.88 | 0.99 |
| `swap_ranges` | 3.20 | 3.30 | 1.03 |
| `transform` | 1.62 | 1.63 | 1.00 |
| **geomean** | **2.16** | **2.16** | **1.00** |

<img src="https://boostedcpp.net/wp-content/uploads/2026/08/bench1_per_algo_myint_1p2s.png" alt="First benchmark: std/seg per algorithm, T = MyInt, shape = 1+2S" class="aligncenter size-full" />

<img src="https://boostedcpp.net/wp-content/uploads/2026/08/bench1_per_algo_myfatint4_1p2s.png" alt="First benchmark: std/seg per algorithm, T = MyFatInt&lt;4&gt;, shape = 1+2S" class="aligncenter size-full" />

<img src="https://boostedcpp.net/wp-content/uploads/2026/08/bench1_per_algo_myfatint8_1p2s.png" alt="First benchmark: std/seg per algorithm, T = MyFatInt&lt;8&gt;, shape = 1+2S" class="aligncenter size-full" />

### Reading the first benchmark

The distribution is bimodal, and the split follows the shape column, not the algorithm:

- The `1S` variants — segmented input, flat output — behave like the first article. The output is a plain `vector` iterator, the leaves are the same flat scans that vectorised so well in part 1: `transform(1S)` 5.14×, `copy(1S)` 4.70×, `swap_ranges(1S)` 4.47×.
- The `2S` variants — flat input, segmented output — are the sore spot. Several of them are *slower* than the flat fallback (`copy_if(2S miss)` 0.68×, `copy_if(2S hit)` 0.92×, `remove_copy(2S hit)` 0.95×), and on individual compilers the damage is bigger: Clang's `copy(2S)` scores 0.67× and its `transform(2S)` 0.70×; GCC's `swap_ranges(2S)` scores 0.75×.

Why does segmenting *only the output* hurt so much, when segmenting only the input helps so much? Looking at the generated assembly, three causes stack up:

1. **The bounded write loop does not vectorise.** With the destination bound live inside the loop there are two loop-carried exit conditions, and both GCC and Clang give up on SIMD for the plain `copy(2S)` leaf; the emitted code is a scalar load-store-compare-branch cycle. MSVC is the exception: its vectoriser handles some of the two-exit loops, which is one reason its `2S` column degrades less.
2. **Conditional writes plus a destination bound is the worst combination.** In `copy_if`/`remove_copy_if` the store address advances *data-dependently* (only on predicate hits), which already rules out straightforward vectorisation — the standard's _"exactly N predicate applications"_ contract also forbids speculative execution tricks, and adding the after-write destination-full test puts a second unpredictable branch in the loop. The result is that the segmented `2S` conditional algorithms hover around 0.8×-1.0× of the flat loop in their straightforward implementation: the segmentation machinery costs, and there is no SIMD to regain the lost performance.
3. **Per-segment bookkeeping is not neglectable.** A maximum 128-element destination segment means the outer machinery (compose/decompose, input-output iterator returns...) runs once per segmented.

The element-type columns also gives use an important information. The `MyFatInt<8>` geomeans converge towards 1.0× on GCC and Clang (1.05×-1.06×): the reads and writes of a `copy` over thousands of 32-byte elements saturate the memory subsystem, the CPU becomes bandwidth-limited, and no amount of iteration optimization can speed up the memory bus. `MyFatInt<4>` (16 bytes) sits in between: part of the working set is cache-resident, so moderate segmentation gains (1.1×-1.9×) can be achieved. It is worth noting that MSVC keeps a 1.38× geomean even on the fat type — not because its segmented code is faster in absolute terms, but because its scalar flat loops are slower, leaving more room below the bandwidth ceiling.

## Second benchmark: taking advantage of random-access leaves

### The optimization

There is an observation that can help the `2S` cases (destination-only segmentation): when the destination is a segmented container like `deque`, the localmost destination iterator is probably a *random-access* iterator (in Boost's `deque` it is a plain `T*` into the current block). The same holds for the source when it is flat or when it has been decomposed down to its blocks. And when both the input and the output leaves are random-access, the two-exit-condition loop (source or destination range exhausted) can be optimized: the leaf knows *up front* how many elements fit.

For the unconditional algorithms (`copy`, `copy_n`, `swap_ranges`, `transform`, and `remove_copy(miss)`-like flows), instead of testing the destination on every element, the leaf computes the number of elements to process once and falls back to the single-exit loop:

```cpp
//Random-access source and destination: precompute the trip count,
//then run the single-exit loop
for( difference_type n = min(last - first, dst_last - dst_first)
   ; n ; ++first, ++dst_first) {
   --n;
   *dst_first = *first;
}
```

One subtraction and one comparison can replace N per-element destination tests, and — more importantly — the inner loop recovers a single-exit shape that can help auto-vectorisers.

For the conditionally-writing algorithms (`copy_if`, `remove_copy`...) the trip count cannot be precomputed (the number of writes is data-dependent), but the check can still be *amortised*: as long as the destination has at least `B` slots free, a block of `B` source elements can be processed with no destination test at all, because even if every predicate hits, the writes fit. If `B` is a power of two, it will probably help the optimizer select better instructions:

```cpp
//Random-access source and destination: if both have room to process
//B elements in the worst case, then the inner loop is greatly simplified
difference_type n = last - first;
while(n >= B && (dst_last - dst_first) >= B) {
   n -= B;
   for(difference_type chunk = B; chunk; --chunk, ++first) {
      if(pred(*first)) {
         *dst_first = *first;
         ++dst_first;
      }
   }
}
//tail: per-element destination checks...
```

The destination test now runs once per `B` elements (e.g. 16 in the current implementation) instead of once per element, the predicate is still applied exactly once per element, and the loop body might be light enough for the compiler to unroll.

### Benchmark results

Same benchmark, same machine, same compilers, with these random-access leaves enabled:

Geomean per compiler, split by shape:

#### Shape `1S` — 1S (segmented input, flat output)

T = `MyInt`:

| Compiler | `nsg/seg` | `std/seg` | `std/nsg` |
| --- | --- | --- | --- |
| GCC 16 | 1.92 | 1.87 | 0.97 |
| Clang 22 | 3.01 | 2.95 | 0.98 |
| MSVC 2026 | 5.29 | 5.27 | 1.00 |

T = `MyFatInt<4>`:

| Compiler | `nsg/seg` | `std/seg` | `std/nsg` |
| --- | --- | --- | --- |
| GCC 16 | 0.99 | 1.05 | 1.06 |
| Clang 22 | 1.41 | 1.18 | 0.84 |
| MSVC 2026 | 2.65 | 2.64 | 0.99 |

T = `MyFatInt<8>`:

| Compiler | `nsg/seg` | `std/seg` | `std/nsg` |
| --- | --- | --- | --- |
| GCC 16 | 1.01 | 1.02 | 1.01 |
| Clang 22 | 1.09 | 1.04 | 0.95 |
| MSVC 2026 | 1.79 | 1.74 | 0.97 |

Per-algorithm for `MyInt`, shape `1S` (geomean of the three compilers; per-compiler breakdowns are in the Annex):

| Algorithm | `nsg/seg` | `std/seg` | `std/nsg` |
| --- | --- | --- | --- |
| `copy` | 4.89 | 4.81 | 0.98 |
| `copy_if(hit)` | 2.08 | 2.08 | 1.00 |
| `copy_if(miss)` | 2.61 | 2.14 | 0.82 |
| `copy_n` | 5.25 | 5.10 | 0.97 |
| `remove_copy(hit)` | 2.48 | 2.55 | 1.02 |
| `remove_copy(miss)` | 2.45 | 2.65 | 1.08 |
| `remove_copy_if(hit)` | 1.90 | 1.89 | 1.00 |
| `remove_copy_if(miss)` | 2.19 | 2.30 | 1.05 |
| `swap_ranges` | 4.68 | 4.67 | 1.00 |
| `transform` | 5.37 | 5.04 | 0.94 |
| **geomean** | **3.13** | **3.07** | **0.98** |

<img src="https://boostedcpp.net/wp-content/uploads/2026/08/bench2_per_algo_myint_1s.png" alt="Second benchmark: std/seg per algorithm, T = MyInt, shape = 1S" class="aligncenter size-full" />

<img src="https://boostedcpp.net/wp-content/uploads/2026/08/bench2_per_algo_myfatint4_1s.png" alt="Second benchmark: std/seg per algorithm, T = MyFatInt&lt;4&gt;, shape = 1S" class="aligncenter size-full" />

<img src="https://boostedcpp.net/wp-content/uploads/2026/08/bench2_per_algo_myfatint8_1s.png" alt="Second benchmark: std/seg per algorithm, T = MyFatInt&lt;8&gt;, shape = 1S" class="aligncenter size-full" />

#### Shape `2S` — 2S (flat input, segmented output)

T = `MyInt`:

| Compiler | `nsg/seg` | `std/seg` | `std/nsg` |
| --- | --- | --- | --- |
| GCC 16 | 2.78 | 2.81 | 1.01 |
| Clang 22 | 2.56 | 2.61 | 1.02 |
| MSVC 2026 | 2.59 | 2.59 | 1.00 |

T = `MyFatInt<4>`:

| Compiler | `nsg/seg` | `std/seg` | `std/nsg` |
| --- | --- | --- | --- |
| GCC 16 | 1.30 | 1.32 | 1.02 |
| Clang 22 | 1.24 | 1.30 | 1.05 |
| MSVC 2026 | 1.55 | 1.47 | 0.95 |

T = `MyFatInt<8>`:

| Compiler | `nsg/seg` | `std/seg` | `std/nsg` |
| --- | --- | --- | --- |
| GCC 16 | 1.03 | 1.03 | 1.00 |
| Clang 22 | 1.03 | 1.03 | 1.00 |
| MSVC 2026 | 1.19 | 1.14 | 0.96 |

Per-algorithm for `MyInt`, shape `2S` (geomean of the three compilers; per-compiler breakdowns are in the Annex):

| Algorithm | `nsg/seg` | `std/seg` | `std/nsg` |
| --- | --- | --- | --- |
| `copy` | 4.99 | 4.99 | 1.00 |
| `copy_if(hit)` | 1.64 | 1.65 | 1.01 |
| `copy_if(miss)` | 1.44 | 1.54 | 1.08 |
| `copy_n` | 5.10 | 5.76 | 1.13 |
| `remove_copy(hit)` | 1.78 | 1.72 | 0.97 |
| `remove_copy(miss)` | 1.62 | 1.60 | 0.99 |
| `remove_copy_if(hit)` | 1.86 | 1.90 | 1.02 |
| `remove_copy_if(miss)` | 1.87 | 1.68 | 0.90 |
| `swap_ranges` | 4.67 | 5.02 | 1.08 |
| `transform` | 5.88 | 5.66 | 0.96 |
| **geomean** | **2.64** | **2.67** | **1.01** |

<img src="https://boostedcpp.net/wp-content/uploads/2026/08/bench2_per_algo_myint_2s.png" alt="Second benchmark: std/seg per algorithm, T = MyInt, shape = 2S" class="aligncenter size-full" />

<img src="https://boostedcpp.net/wp-content/uploads/2026/08/bench2_per_algo_myfatint4_2s.png" alt="Second benchmark: std/seg per algorithm, T = MyFatInt&lt;4&gt;, shape = 2S" class="aligncenter size-full" />

<img src="https://boostedcpp.net/wp-content/uploads/2026/08/bench2_per_algo_myfatint8_2s.png" alt="Second benchmark: std/seg per algorithm, T = MyFatInt&lt;8&gt;, shape = 2S" class="aligncenter size-full" />

#### Shape `1+2S` — 1+2S (segmented input and output)

T = `MyInt`:

| Compiler | `nsg/seg` | `std/seg` | `std/nsg` |
| --- | --- | --- | --- |
| GCC 16 | 3.24 | 3.24 | 1.00 |
| Clang 22 | 4.02 | 3.89 | 0.97 |
| MSVC 2026 | 6.67 | 6.45 | 0.97 |

T = `MyFatInt<4>`:

| Compiler | `nsg/seg` | `std/seg` | `std/nsg` |
| --- | --- | --- | --- |
| GCC 16 | 1.53 | 1.58 | 1.03 |
| Clang 22 | 1.56 | 1.62 | 1.04 |
| MSVC 2026 | 2.59 | 2.06 | 0.79 |

T = `MyFatInt<8>`:

| Compiler | `nsg/seg` | `std/seg` | `std/nsg` |
| --- | --- | --- | --- |
| GCC 16 | 1.09 | 1.09 | 1.00 |
| Clang 22 | 1.16 | 1.16 | 1.00 |
| MSVC 2026 | 1.81 | 1.28 | 0.71 |

Per-algorithm for `MyInt`, shape `1+2S` (geomean of the three compilers; per-compiler breakdowns are in the Annex):

| Algorithm | `nsg/seg` | `std/seg` | `std/nsg` |
| --- | --- | --- | --- |
| `copy` | 6.44 | 6.15 | 0.96 |
| `copy_if(hit)` | 4.34 | 4.35 | 1.00 |
| `copy_if(miss)` | 4.50 | 4.36 | 0.97 |
| `copy_n` | 6.69 | 7.06 | 1.06 |
| `remove_copy(hit)` | 3.04 | 3.15 | 1.04 |
| `remove_copy(miss)` | 3.02 | 3.00 | 1.00 |
| `remove_copy_if(hit)` | 3.38 | 3.16 | 0.93 |
| `remove_copy_if(miss)` | 3.03 | 2.99 | 0.99 |
| `swap_ranges` | 5.48 | 4.82 | 0.88 |
| `transform` | 6.72 | 6.54 | 0.97 |
| **geomean** | **4.43** | **4.33** | **0.98** |

<img src="https://boostedcpp.net/wp-content/uploads/2026/08/bench2_per_algo_myint_1p2s.png" alt="Second benchmark: std/seg per algorithm, T = MyInt, shape = 1+2S" class="aligncenter size-full" />

<img src="https://boostedcpp.net/wp-content/uploads/2026/08/bench2_per_algo_myfatint4_1p2s.png" alt="Second benchmark: std/seg per algorithm, T = MyFatInt&lt;4&gt;, shape = 1+2S" class="aligncenter size-full" />

<img src="https://boostedcpp.net/wp-content/uploads/2026/08/bench2_per_algo_myfatint8_1p2s.png" alt="Second benchmark: std/seg per algorithm, T = MyFatInt&lt;8&gt;, shape = 1+2S" class="aligncenter size-full" />

### Reading the random-access iterator optimized benchmark

Good news: the `MyInt` geomean moves from 1.94× to 3.29×, and the previously identified weak points improve:

- **The unconditional `2S` (segmented output) cases are fixed.** Clang's `copy(2S)` goes from 0.67× to 4.42×, its `transform(2S)` from 0.70× to 5.76×; GCC's `swap_ranges(2S)` from 0.75× to 3.99×, its `copy_n(2S)` from 0.85× to 4.14×. The generated assembly confirms the mechanism: with the trip count precomputed, the leaf loop compiles to the same SIMD body as the `1S` case. The `2S` rows are now statistically indistinguishable from `1S` rows — segmenting the output has become as profitable as segmenting the input.
- **The `1+2S` cases (segmented input+output) improve the most in absolute terms** (`copy(1+2S)` 3.54× → 6.15×, `transform(1+2S)` 1.63× → 6.54×), because both decompositions now bottom out in a single flat vectorised loop per (source-block × destination-block) intersection, and the intersections are computed with subtractions, not per-element tests.
- **The `1S` (segmented input) rows are unaffected**, as expected: they never executed the bounded leaves in the first place.
- **The conditional algorithms improve, but still behind.** `copy_if(1+2S hit)` more than doubles from 1.99× to 4.35× (MSVC reaches 9.8×) and `remove_copy_if(1+2S)` lands around 3×, thanks to the block-amortised destination check. But `copy_if(2S hit)` at 1.65× is still far from `copy(2S)` at 4.99×: the data-dependent store position remains, so the block loop is faster scalar code, not vectorised code. That gap is not a segmentation problem — flat `std::copy_if` has the same limitation — it is the price of the predicate contract.

On the fat types the improvements are real but bounded by the memory bandwidth: `MyFatInt<4>` climbs from 1.11×/1.17×/1.94× to 1.30×/1.35×/2.00× (GCC/Clang/MSVC), and `MyFatInt<8>` doesn't move at all (1.04×/1.07×/1.36×). Once every mode saturates the bus, executing fewer instructions barely shortens the waiting.

## Final conclusions

Let's see the geomean of `std/seg` (Boost segmented algorithm is X times faster than `std`) across all 30 sub-benchmarks for each compiler, comparing the forward-iterator leaves of the first benchmark with the random-access leaves of the second, one chart per value type:

<img src="https://boostedcpp.net/wp-content/uploads/2026/08/conclusions_std_seg_myint.png" alt="Geomean std/seg per compiler, T = MyInt" class="aligncenter size-full" />

<img src="https://boostedcpp.net/wp-content/uploads/2026/08/conclusions_std_seg_myfatint4.png" alt="Geomean std/seg per compiler, T = MyFatInt&lt;4&gt;" class="aligncenter size-full" />

<img src="https://boostedcpp.net/wp-content/uploads/2026/08/conclusions_std_seg_myfatint8.png" alt="Geomean std/seg per compiler, T = MyFatInt&lt;8&gt;" class="aligncenter size-full" />

Some findings that are worth carrying forward from this analysis:

1. **Austern's decomposition can be extended to output ranges, but it needs special handling and performance can suffer.** A bounded, two-exit-condition write loop defeats today's auto-vectorisers, and it can run *slower* than a flat loop. Input segmentation is essentially free to exploit; output segmentation has to be specially treated.
2. **Random-access optimizations are important.** When the localmost iterators on both sides are random-access, precomputing the trip count for unconditional algorithms, and block-amortising the destination check for conditional ones, yields a compiler-friendly inner loop.
3. **The standard's predicate-call contract has a measurable impact.** Exactly-N predicate applications forbid speculative or re-scanning implementations, forcing leaves to report input-consumption and output-position precisely upward, and keeping the conditionally-writing algorithms less optimizable. A hypothetical relaxed contract would open the door to better-performing algorithms.
4. **Segmented gains are not universal**: for some types and algorithms, memory bandwidth is a hard limit that can't be surpassed. Segmented algorithms shine where memory pressure is low and the  work is instruction-bound.

In the first article we ended by noting that Austern's abstraction *ages into* hardware improvements. This second article adds a nuance: the abstraction is also restricted by *interface* decisions. The STL's unbounded output iterators are a good simplification, but they might need novel improvements to Austern's pattern to fully exploit segmented algorithm opportunities. Shown speedups are very encouraging and a very good reason to keep digging.

---

## Annex: Per-compiler results

Each compiler section shows one chart per value type: 30 sub-benchmarks on the X axis and paired bars for the first and second benchmark configurations (`std/seg` ratio). The Y axis is not shared between charts — the goal is to see each compiler's internal ranking and the first-vs-second contrast. The tables list every sub-benchmark for `T = MyInt` (B1 = first benchmark, B2 = second benchmark).

### GCC 16

<img src="https://boostedcpp.net/wp-content/uploads/2026/08/annex_gcc16_myint.png" alt="GCC 16 per-algorithm results, T = MyInt" class="aligncenter size-full" />

<img src="https://boostedcpp.net/wp-content/uploads/2026/08/annex_gcc16_myfatint4.png" alt="GCC 16 per-algorithm results, T = MyFatInt&lt;4&gt;" class="aligncenter size-full" />

<img src="https://boostedcpp.net/wp-content/uploads/2026/08/annex_gcc16_myfatint8.png" alt="GCC 16 per-algorithm results, T = MyFatInt&lt;8&gt;" class="aligncenter size-full" />

GCC is the compiler that gains the most *relative* ground from the random-access leaves on `MyInt`: the geomean doubles (1.31× → 2.57×). Without them, GCC only keeps the `1S` store-heavy loops (`transform(1S)` 3.55×, `swap_ranges(1S)` 3.49×) and everything touching a segmented destination sits at 0.68×-1.4×. With them, the whole `copy`/`copy_n`/`swap_ranges`/`transform` family lands in the 3.2×-7.1× band (`copy_n(1+2S)` 7.06×, `transform(1+2S)` 6.82×). The conditional algorithms settle at 1.8×-3.1× — GCC does not if-convert the blocked `copy_if` body, so the predicate branch stays, but the amortised destination check still pays. On `MyFatInt<4>` the geomean moves 1.11× → 1.30×; on `MyFatInt<8>` nothing moves — pure bandwidth.

| Algorithm | `nsg/seg` B1 | `std/seg` B1 | `std/nsg` B1 | `nsg/seg` B2 | `std/seg` B2 | `std/nsg` B2 |
| --- | --- | --- | --- | --- | --- | --- |
| `copy(1S)` | 2.58 | 2.78 | 1.08 | 3.32 | 3.17 | 0.95 |
| `copy(2S)` | 1.11 | 1.11 | 1.00 | 4.33 | 4.34 | 1.00 |
| `copy(1+2S)` | 1.11 | 1.26 | 1.14 | 5.59 | 5.32 | 0.95 |
| `copy_if(1S hit)` | 1.00 | 1.01 | 1.01 | 1.15 | 1.08 | 0.94 |
| `copy_if(2S hit)` | 1.20 | 1.16 | 0.97 | 2.60 | 2.74 | 1.06 |
| `copy_if(1+2S hit)` | 2.02 | 1.75 | 0.87 | 3.01 | 3.06 | 1.02 |
| `copy_if(1S miss)` | 1.12 | 1.10 | 0.99 | 0.89 | 0.92 | 1.03 |
| `copy_if(2S miss)` | 0.78 | 0.68 | 0.87 | 1.68 | 1.81 | 1.08 |
| `copy_if(1+2S miss)` | 1.55 | 1.39 | 0.89 | 2.20 | 2.05 | 0.93 |
| `copy_n(1S)` | 0.92 | 0.88 | 0.95 | 4.51 | 4.32 | 0.96 |
| `copy_n(2S)` | 0.94 | 0.85 | 0.91 | 4.46 | 4.14 | 0.93 |
| `copy_n(1+2S)` | 1.38 | 1.67 | 1.21 | 6.30 | 7.06 | 1.12 |
| `remove_copy(1S hit)` | 1.22 | 1.51 | 1.24 | 1.72 | 1.31 | 0.76 |
| `remove_copy(2S hit)` | 1.40 | 1.36 | 0.97 | 2.24 | 2.18 | 0.97 |
| `remove_copy(1+2S hit)` | 1.04 | 1.06 | 1.02 | 2.03 | 2.24 | 1.10 |
| `remove_copy(1S miss)` | 1.19 | 1.53 | 1.29 | 1.47 | 1.24 | 0.84 |
| `remove_copy(2S miss)` | 1.19 | 1.17 | 0.98 | 1.81 | 1.85 | 1.02 |
| `remove_copy(1+2S miss)` | 1.52 | 1.49 | 0.98 | 1.98 | 1.94 | 0.98 |
| `remove_copy_if(1S hit)` | 1.01 | 0.98 | 0.96 | 0.97 | 1.14 | 1.18 |
| `remove_copy_if(2S hit)` | 1.11 | 1.13 | 1.01 | 1.93 | 1.99 | 1.03 |
| `remove_copy_if(1+2S hit)` | 1.33 | 1.26 | 0.95 | 2.15 | 2.04 | 0.95 |
| `remove_copy_if(1S miss)` | 1.53 | 1.53 | 1.00 | 1.28 | 1.52 | 1.19 |
| `remove_copy_if(2S miss)` | 1.21 | 1.35 | 1.12 | 2.07 | 2.01 | 0.97 |
| `remove_copy_if(1+2S miss)` | 1.32 | 1.34 | 1.02 | 2.17 | 2.10 | 0.97 |
| `swap_ranges(1S)` | 3.47 | 3.49 | 1.01 | 3.46 | 3.97 | 1.15 |
| `swap_ranges(2S)` | 0.75 | 0.75 | 1.00 | 3.96 | 3.99 | 1.01 |
| `swap_ranges(1+2S)` | 1.16 | 1.07 | 0.92 | 4.28 | 4.25 | 0.99 |
| `transform(1S)` | 3.98 | 3.55 | 0.89 | 4.04 | 3.40 | 0.84 |
| `transform(2S)` | 0.95 | 0.92 | 0.97 | 5.17 | 5.28 | 1.02 |
| `transform(1+2S)` | 1.36 | 1.50 | 1.10 | 6.84 | 6.82 | 1.00 |
| **geomean** | **1.30** | **1.31** | **1.01** | **2.59** | **2.57** | **0.99** |

### Clang 22

<img src="https://boostedcpp.net/wp-content/uploads/2026/08/annex_clang22_myint.png" alt="Clang 22 per-algorithm results, T = MyInt" class="aligncenter size-full" />

<img src="https://boostedcpp.net/wp-content/uploads/2026/08/annex_clang22_myfatint4.png" alt="Clang 22 per-algorithm results, T = MyFatInt&lt;4&gt;" class="aligncenter size-full" />

<img src="https://boostedcpp.net/wp-content/uploads/2026/08/annex_clang22_myfatint8.png" alt="Clang 22 per-algorithm results, T = MyFatInt&lt;8&gt;" class="aligncenter size-full" />

Clang shows the starkest before/after contrast, because its vectoriser is both the most eager and the most easily blocked. In the first benchmark it produces the best `1S` numbers of the matrix (`transform(1S)` 7.17×, `copy(1S)` 5.85×) *and* the worst `2S` numbers (`copy(2S)` 0.67×, `remove_copy(2S miss)` 0.69×, `transform(2S)` 0.70×) — the bounded write loop completely disables its SIMD engine. With the random-access leaves the same rows read 4.42×, 1.53× and 5.76×, and `transform` posts 5.8×-7.4× across all three shapes. An interesting oddity: `copy_if(1S miss)` shows `std/nsg` ≈ 0.5, meaning libstdc++'s `std::copy_if` is twice as fast as the plain flat loop on a miss-heavy scan — libstdc++ hand-hoists the write out of the branch in a way Clang rewards; the segmented path matches that trick only partially. On `MyFatInt<4>` the geomean moves 1.17× → 1.35×, on `MyFatInt<8>` 1.06× → 1.07×: the bandwidth ceiling again.

| Algorithm | `nsg/seg` B1 | `std/seg` B1 | `std/nsg` B1 | `nsg/seg` B2 | `std/seg` B2 | `std/nsg` B2 |
| --- | --- | --- | --- | --- | --- | --- |
| `copy(1S)` | 5.77 | 5.85 | 1.01 | 5.94 | 5.92 | 1.00 |
| `copy(2S)` | 0.67 | 0.67 | 1.00 | 4.42 | 4.42 | 1.00 |
| `copy(1+2S)` | 6.85 | 6.58 | 0.96 | 6.99 | 6.99 | 1.00 |
| `copy_if(1S hit)` | 1.67 | 1.59 | 0.96 | 1.65 | 1.74 | 1.05 |
| `copy_if(2S hit)` | 0.80 | 0.80 | 0.99 | 1.28 | 1.23 | 0.96 |
| `copy_if(1+2S hit)` | 1.49 | 1.49 | 1.00 | 2.77 | 2.74 | 0.99 |
| `copy_if(1S miss)` | 5.29 | 2.38 | 0.45 | 4.58 | 2.45 | 0.53 |
| `copy_if(2S miss)` | 0.87 | 0.69 | 0.79 | 1.88 | 2.01 | 1.07 |
| `copy_if(1+2S miss)` | 2.00 | 2.05 | 1.03 | 5.82 | 5.70 | 0.98 |
| `copy_n(1S)` | 5.36 | 5.16 | 0.96 | 5.45 | 5.21 | 0.96 |
| `copy_n(2S)` | 0.59 | 0.91 | 1.55 | 4.99 | 7.79 | 1.56 |
| `copy_n(1+2S)` | 6.26 | 6.52 | 1.04 | 7.78 | 8.15 | 1.05 |
| `remove_copy(1S hit)` | 1.73 | 2.31 | 1.34 | 1.64 | 2.32 | 1.41 |
| `remove_copy(2S hit)` | 0.83 | 0.79 | 0.95 | 1.46 | 1.40 | 0.96 |
| `remove_copy(1+2S hit)` | 1.04 | 1.01 | 0.97 | 2.12 | 2.14 | 1.01 |
| `remove_copy(1S miss)` | 1.53 | 2.32 | 1.52 | 1.53 | 2.30 | 1.51 |
| `remove_copy(2S miss)` | 0.70 | 0.69 | 0.98 | 1.56 | 1.53 | 0.98 |
| `remove_copy(1+2S miss)` | 0.99 | 0.95 | 0.97 | 2.06 | 2.07 | 1.01 |
| `remove_copy_if(1S hit)` | 1.83 | 1.82 | 0.99 | 1.78 | 1.56 | 0.88 |
| `remove_copy_if(2S hit)` | 0.78 | 0.81 | 1.04 | 1.44 | 1.45 | 1.01 |
| `remove_copy_if(1+2S hit)` | 1.05 | 1.23 | 1.17 | 2.66 | 2.47 | 0.93 |
| `remove_copy_if(1S miss)` | 1.59 | 1.54 | 0.97 | 1.59 | 1.55 | 0.98 |
| `remove_copy_if(2S miss)` | 0.79 | 1.33 | 1.68 | 2.39 | 1.82 | 0.76 |
| `remove_copy_if(1+2S miss)` | 1.18 | 1.13 | 0.96 | 2.19 | 2.17 | 0.99 |
| `swap_ranges(1S)` | 4.93 | 4.32 | 0.88 | 5.01 | 4.33 | 0.87 |
| `swap_ranges(2S)` | 0.90 | 1.11 | 1.22 | 4.37 | 5.38 | 1.23 |
| `swap_ranges(1+2S)` | 6.22 | 7.99 | 1.28 | 6.72 | 5.00 | 0.74 |
| `transform(1S)` | 7.07 | 7.17 | 1.01 | 7.01 | 6.89 | 0.98 |
| `transform(2S)` | 0.80 | 0.70 | 0.87 | 6.61 | 5.76 | 0.87 |
| `transform(1+2S)` | 0.88 | 0.88 | 1.00 | 7.43 | 7.44 | 1.00 |
| **geomean** | **1.67** | **1.71** | **1.03** | **3.14** | **3.11** | **0.99** |

### MSVC 2026 (toolset v14.51)

<img src="https://boostedcpp.net/wp-content/uploads/2026/08/annex_msvc145_myint.png" alt="MSVC 2026 per-algorithm results, T = MyInt" class="aligncenter size-full" />

<img src="https://boostedcpp.net/wp-content/uploads/2026/08/annex_msvc145_myfatint4.png" alt="MSVC 2026 per-algorithm results, T = MyFatInt&lt;4&gt;" class="aligncenter size-full" />

<img src="https://boostedcpp.net/wp-content/uploads/2026/08/annex_msvc145_myfatint8.png" alt="MSVC 2026 per-algorithm results, T = MyFatInt&lt;8&gt;" class="aligncenter size-full" />

MSVC starts from the highest first-benchmark baseline (3.28× geomean on `MyInt`) for two reasons: its v14.51 vectoriser survives some of the bounded write loops that stop GCC and Clang cold, and its scalar flat loops are comparatively slow, which inflates every `X/seg` ratio. Still, the random-access leaves lift it to 4.45×, and the conditional algorithms respond more dramatically here than anywhere else: `copy_if(1+2S hit)` jumps from 3.02× to 9.84× and `copy_if(1+2S miss)` from 2.87× to 7.10× — the v14.51 back-end if-converts the blocked predicate body into branchless code once the destination check leaves the loop. The lingering weak spot is the flat-source `2S` family (`remove_copy(2S)`, `copy_if(2S)` around 1.0×-1.7×), where the Microsoft STL's own loops are already reasonable and the deque write side dominates. MSVC is also the only compiler that keeps meaningful gains on `MyFatInt<8>` (1.38× and 1.36× geomean, essentially unchanged between benchmarks): its scalar baseline is far enough from the bandwidth ceiling that per-block iteration still shows.

| Algorithm | `nsg/seg` B1 | `std/seg` B1 | `std/nsg` B1 | `nsg/seg` B2 | `std/seg` B2 | `std/nsg` B2 |
| --- | --- | --- | --- | --- | --- | --- |
| `copy(1S)` | 6.37 | 6.38 | 1.00 | 5.94 | 5.94 | 1.00 |
| `copy(2S)` | 5.87 | 5.88 | 1.00 | 6.49 | 6.49 | 1.00 |
| `copy(1+2S)` | 5.83 | 5.37 | 0.92 | 6.82 | 6.25 | 0.92 |
| `copy_if(1S hit)` | 4.11 | 4.13 | 1.00 | 4.76 | 4.77 | 1.00 |
| `copy_if(2S hit)` | 0.84 | 0.84 | 1.01 | 1.33 | 1.33 | 1.00 |
| `copy_if(1+2S hit)` | 3.02 | 3.02 | 1.00 | 9.83 | 9.84 | 1.00 |
| `copy_if(1S miss)` | 4.30 | 4.31 | 1.00 | 4.34 | 4.35 | 1.00 |
| `copy_if(2S miss)` | 0.68 | 0.66 | 0.97 | 0.94 | 1.01 | 1.08 |
| `copy_if(1+2S miss)` | 2.87 | 2.87 | 1.00 | 7.10 | 7.10 | 1.00 |
| `copy_n(1S)` | 5.04 | 5.04 | 1.00 | 5.89 | 5.89 | 1.00 |
| `copy_n(2S)` | 5.22 | 5.19 | 0.99 | 5.96 | 5.94 | 1.00 |
| `copy_n(1+2S)` | 4.63 | 4.63 | 1.00 | 6.10 | 6.11 | 1.00 |
| `remove_copy(1S hit)` | 5.92 | 5.92 | 1.00 | 5.44 | 5.44 | 1.00 |
| `remove_copy(2S hit)` | 0.76 | 0.81 | 1.07 | 1.72 | 1.68 | 0.98 |
| `remove_copy(1+2S hit)` | 3.40 | 3.27 | 0.96 | 6.55 | 6.54 | 1.00 |
| `remove_copy(1S miss)` | 6.50 | 6.51 | 1.00 | 6.50 | 6.50 | 1.00 |
| `remove_copy(2S miss)` | 1.27 | 1.29 | 1.01 | 1.50 | 1.46 | 0.97 |
| `remove_copy(1+2S miss)` | 4.77 | 4.77 | 1.00 | 6.75 | 6.75 | 1.00 |
| `remove_copy_if(1S hit)` | 3.57 | 3.57 | 1.00 | 3.99 | 3.82 | 0.96 |
| `remove_copy_if(2S hit)` | 1.05 | 1.10 | 1.05 | 2.31 | 2.36 | 1.02 |
| `remove_copy_if(1+2S hit)` | 2.08 | 1.99 | 0.95 | 6.77 | 6.25 | 0.92 |
| `remove_copy_if(1S miss)` | 5.09 | 5.22 | 1.03 | 5.19 | 5.19 | 1.00 |
| `remove_copy_if(2S miss)` | 1.18 | 1.16 | 0.98 | 1.32 | 1.30 | 0.98 |
| `remove_copy_if(1+2S miss)` | 4.41 | 4.41 | 1.00 | 5.87 | 5.87 | 1.00 |
| `swap_ranges(1S)` | 5.85 | 5.93 | 1.01 | 5.91 | 5.92 | 1.00 |
| `swap_ranges(2S)` | 4.89 | 4.89 | 1.00 | 5.89 | 5.89 | 1.00 |
| `swap_ranges(1+2S)` | 4.56 | 4.19 | 0.92 | 5.73 | 5.26 | 0.92 |
| `transform(1S)` | 5.34 | 5.34 | 1.00 | 5.48 | 5.47 | 1.00 |
| `transform(2S)` | 4.97 | 4.98 | 1.00 | 5.95 | 5.95 | 1.00 |
| `transform(1+2S)` | 3.58 | 3.29 | 0.92 | 5.98 | 5.51 | 0.92 |
| **geomean** | **3.30** | **3.28** | **0.99** | **4.50** | **4.45** | **0.99** |
