#!/usr/bin/env python3
"""Build a shadow segmented_search.hpp that keeps the unified dispatch but
routes only SEGMENTED haystacks through mismatch; non-segmented haystacks get
the flat lock-step verify (HEAD's shape).  Everything else is untouched."""
import sys, os, re

src, dst = sys.argv[1], sys.argv[2]
t = open(src).read()

# 1) mismatch overload: constrain to a segmented haystack.
old_mm = """//Homogeneous ends: reuse mismatch (recursive on both ranges, RA leaf).
template <class FwdIt1, class FwdIt2, class Tag, class Cat>
BOOST_CONTAINER_FORCEINLINE segduo<FwdIt1, FwdIt2>
segmented_search_verify
   (FwdIt1 it, FwdIt1 last, FwdIt2 s_it, FwdIt2 s_last,
    Tag tag, Cat cat)
{
   return (segmented_mismatch_bounded_dispatch)
      (it, last, s_it, s_last, mismatch_equal(), tag, cat);
}"""
new_mm = """//Segmented haystack with homogeneous ends: reuse mismatch.
template <class FwdIt1, class FwdIt2, class Tag, class Cat>
BOOST_CONTAINER_FORCEINLINE
typename algo_enable_if_c<Tag::value, segduo<FwdIt1, FwdIt2> >::type
segmented_search_verify
   (FwdIt1 it, FwdIt1 last, FwdIt2 s_it, FwdIt2 s_last,
    Tag tag, Cat cat)
{
   return (segmented_mismatch_bounded_dispatch)
      (it, last, s_it, s_last, mismatch_equal(), tag, cat);
}"""
assert old_mm in t, "mismatch overload not found"
t = t.replace(old_mm, new_mm)

# 2) flat overload: also take non-segmented haystacks, not just sentinels.
old_flat = """typename algo_enable_if_c
   <is_sentinel<Sent1, FwdIt1>::value || is_sentinel<Sent2, FwdIt2>::value
   , segduo<FwdIt1, FwdIt2> >::type
segmented_search_verify
   (FwdIt1 it, Sent1 last, FwdIt2 s_it, Sent2 s_last, Tag, Cat)"""
new_flat = """typename algo_enable_if_c
   <!Tag::value || is_sentinel<Sent1, FwdIt1>::value
              || is_sentinel<Sent2, FwdIt2>::value
   , segduo<FwdIt1, FwdIt2> >::type
segmented_search_verify
   (FwdIt1 it, Sent1 last, FwdIt2 s_it, Sent2 s_last, Tag, Cat)"""
assert old_flat in t, "flat overload not found"
t = t.replace(old_flat, new_flat)

os.makedirs(os.path.dirname(dst), exist_ok=True)
open(dst, "w").write(t)
print("wrote", dst)
