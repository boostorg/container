#!/usr/bin/env python3
# Generates shadow copies of segmented_find_end.hpp whose verify leaf carries an
# extra bool, in several shapes and polarities.  include/ is never touched:
# every variant lives in its own include root placed before the boost root.

import os, sys

ROOT = "/mnt/d/Data/LocalGit/boost"
HDR = ROOT + "/libs/container/include/boost/container/experimental/segmented_find_end.hpp"
OUT = ROOT + "/libs/container/experimental/cursor_build/g44/findend"

#---------------------------------------------------------------- leaf ------

LEAF_BASE = """typename algo_enable_if_c<!Tag::value, segduo<FwdIt1, FwdIt2> >::type
find_end_verify
   (FwdIt1 it, FwdIt1 last, FwdIt2 s_it, Sent2 s_last, BinaryPred pred, Tag)
{
   BOOST_CONTAINER_SEGMENTED_UNROLL(4)
   for(; it != last; ++it) {
      if(s_it == s_last)
         break;
      if(!pred(*it, *s_it))
         break;
      ++s_it;
   }
   return segduo<FwdIt1, FwdIt2>(it, s_it);
}"""

# a: plain extra field, one return per exit, flag == "caller must stop"
LEAF_A = """typename algo_enable_if_c<!Tag::value, segtrio<FwdIt1, FwdIt2, bool> >::type
find_end_verify
   (FwdIt1 it, FwdIt1 last, FwdIt2 s_it, Sent2 s_last, BinaryPred pred, Tag)
{
   typedef segtrio<FwdIt1, FwdIt2, bool> result_t;

   BOOST_CONTAINER_SEGMENTED_UNROLL(4)
   for(; it != last; ++it) {
      if(s_it == s_last)
         return result_t(it, s_it, true);
      if(!pred(*it, *s_it))
         return result_t(it, s_it, true);
      ++s_it;
   }
   return result_t(it, s_it, s_it == s_last);
}"""

# b: same field, opposite polarity, flag == "caller may resume in next segment"
LEAF_B = """typename algo_enable_if_c<!Tag::value, segtrio<FwdIt1, FwdIt2, bool> >::type
find_end_verify
   (FwdIt1 it, FwdIt1 last, FwdIt2 s_it, Sent2 s_last, BinaryPred pred, Tag)
{
   typedef segtrio<FwdIt1, FwdIt2, bool> result_t;

   BOOST_CONTAINER_SEGMENTED_UNROLL(4)
   for(; it != last; ++it) {
      if(s_it == s_last)
         return result_t(it, s_it, false);
      if(!pred(*it, *s_it))
         return result_t(it, s_it, false);
      ++s_it;
   }
   return result_t(it, s_it, s_it != s_last);
}"""

# c: house single-return / goto out_path shape, flag materialised
LEAF_C = """typename algo_enable_if_c<!Tag::value, segtrio<FwdIt1, FwdIt2, bool> >::type
find_end_verify
   (FwdIt1 it, FwdIt1 last, FwdIt2 s_it, Sent2 s_last, BinaryPred pred, Tag)
{
   typedef segtrio<FwdIt1, FwdIt2, bool> result_t;

   bool stop = true;

   BOOST_CONTAINER_SEGMENTED_UNROLL(4)
   for(; it != last; ++it) {
      if(s_it == s_last)
         goto out_path;
      if(!pred(*it, *s_it))
         goto out_path;
      ++s_it;
   }
   stop = s_it == s_last;

   out_path:
   return result_t(it, s_it, stop);
}"""

# d: baseline loop untouched, flag re-derived from the iterators at the exit
LEAF_D = """typename algo_enable_if_c<!Tag::value, segtrio<FwdIt1, FwdIt2, bool> >::type
find_end_verify
   (FwdIt1 it, FwdIt1 last, FwdIt2 s_it, Sent2 s_last, BinaryPred pred, Tag)
{
   BOOST_CONTAINER_SEGMENTED_UNROLL(4)
   for(; it != last; ++it) {
      if(s_it == s_last)
         break;
      if(!pred(*it, *s_it))
         break;
      ++s_it;
   }
   return segtrio<FwdIt1, FwdIt2, bool>(it, s_it, it != last || s_it == s_last);
}"""

#-------------------------------------------------------------- walker ------

WALK_SIG_BASE = "segduo<SegIt, FwdIt2> find_end_verify"
WALK_SIG_FLAG = "segtrio<SegIt, FwdIt2, bool> find_end_verify"

WALK_TYPEDEF_BASE = """   typedef segduo<SegIt, FwdIt2>          return_t;
   typedef segduo<local_iterator, FwdIt2> local_return_t;"""
WALK_TYPEDEF_FLAG = """   typedef segtrio<SegIt, FwdIt2, bool>          return_t;
   typedef segtrio<local_iterator, FwdIt2, bool> local_return_t;"""

WALK_FIRST_BASE = """         if(BOOST_UNLIKELY(r.first != le || s_it == s_last))
            return return_t(traits::compose(sfirst, r.first), s_it);
         if(BOOST_UNLIKELY(last_seg))
            return return_t(last, s_it);"""

# straight flag test, both exits kept
WALK_FIRST_A = """         if(BOOST_UNLIKELY(r.third))
            return return_t(traits::compose(sfirst, r.first), s_it, true);
         if(BOOST_UNLIKELY(last_seg))
            return return_t(last, s_it, false);"""

WALK_FIRST_B = """         if(BOOST_UNLIKELY(!r.third))
            return return_t(traits::compose(sfirst, r.first), s_it, false);
         if(BOOST_UNLIKELY(last_seg))
            return return_t(last, s_it, true);"""

# e: partition_copy-style fold, the two exits share one site
WALK_FIRST_E = """         if(BOOST_UNLIKELY(r.third || last_seg))
            return return_t(traits::compose(sfirst, r.first), s_it, r.third);"""

WALK_MID_BASE = """         if(BOOST_UNLIKELY(r.first != me || s_it == s_last))
            return return_t(traits::compose(sfirst, r.first), s_it);"""

WALK_MID_A = """         if(BOOST_UNLIKELY(r.third))
            return return_t(traits::compose(sfirst, r.first), s_it, true);"""

WALK_MID_B = """         if(BOOST_UNLIKELY(!r.third))
            return return_t(traits::compose(sfirst, r.first), s_it, false);"""

CALL_BASE = "const segduo<FwdIt1, FwdIt2> r = (find_end_verify)"
CALL_FLAG = "const segtrio<FwdIt1, FwdIt2, bool> r = (find_end_verify)"

#                leaf      first-site      mid-site
VARIANTS = {
   "a": (LEAF_A, WALK_FIRST_A, WALK_MID_A),
   "b": (LEAF_B, WALK_FIRST_B, WALK_MID_B),
   "c": (LEAF_C, WALK_FIRST_A, WALK_MID_A),
   "d": (LEAF_D, WALK_FIRST_A, WALK_MID_A),
   "e": (LEAF_A, WALK_FIRST_E, WALK_MID_A),
}


def once(src, pat, what):
   if src.count(pat) != 1:
      sys.exit("pattern %s found %d times" % (what, src.count(pat)))


def main():
   base = open(HDR).read().replace("\r\n", "\n")
   for pat, what in ((LEAF_BASE, "leaf"), (WALK_SIG_BASE, "walker sig"),
                     (WALK_TYPEDEF_BASE, "walker typedefs"),
                     (WALK_FIRST_BASE, "first site"), (WALK_MID_BASE, "mid site")):
      once(base, pat, what)
   if base.count(CALL_BASE) != 2:
      sys.exit("expected 2 verify call sites in find_end_scan")

   d = "%s/inc_base/boost/container/experimental" % OUT
   os.makedirs(d, exist_ok=True)
   open(d + "/segmented_find_end.hpp", "w").write(base)

   for name in VARIANTS:
      leaf, first, mid = VARIANTS[name]
      out = (base.replace(LEAF_BASE, leaf)
                 .replace(WALK_SIG_BASE, WALK_SIG_FLAG)
                 .replace(WALK_TYPEDEF_BASE, WALK_TYPEDEF_FLAG)
                 .replace(WALK_FIRST_BASE, first)
                 .replace(WALK_MID_BASE, mid)
                 .replace(CALL_BASE, CALL_FLAG))
      d = "%s/inc_%s/boost/container/experimental" % (OUT, name)
      os.makedirs(d, exist_ok=True)
      open(d + "/segmented_find_end.hpp", "w").write(out)

   print("variants written:", " ".join(sorted(VARIANTS)))


if __name__ == "__main__":
   main()
