//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2025-2026. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////

#include <boost/container/experimental/segmented_copy_if.hpp>
#include <boost/core/lightweight_test.hpp>
#include "segmented_test_helper.hpp"
#include <boost/container/vector.hpp>

using namespace boost::container;

struct is_even
{
   bool operator()(int x) const { return x % 2 == 0; }
};

void test_copy_if_full_range()
{
   test_detail::seg_vector<int> sv;
   int a1[] = {1, 2, 3};
   int a2[] = {4, 5};
   int a3[] = {6, 7, 8, 9};
   sv.add_segment_range(a1, a1 + 3);
   sv.add_segment_range(a2, a2 + 2);
   sv.add_segment_range(a3, a3 + 4);

   boost::container::vector<int> out(9, 0);
   boost::container::vector<int>::iterator result =
      segmented_copy_if(sv.begin(), sv.end(), out.begin(), is_even());

   std::size_t count = static_cast<std::size_t>(result - out.begin());
   BOOST_TEST_EQ(count, 4u);
   BOOST_TEST_EQ(out[0], 2);
   BOOST_TEST_EQ(out[1], 4);
   BOOST_TEST_EQ(out[2], 6);
   BOOST_TEST_EQ(out[3], 8);
}

void test_copy_if_none_match()
{
   test_detail::seg_vector<int> sv;
   int a1[] = {1, 3, 5};
   sv.add_segment_range(a1, a1 + 3);

   boost::container::vector<int> out(3, 0);
   boost::container::vector<int>::iterator result =
      segmented_copy_if(sv.begin(), sv.end(), out.begin(), is_even());

   BOOST_TEST(result == out.begin());
}

void test_copy_if_all_match()
{
   test_detail::seg_vector<int> sv;
   int a1[] = {2, 4};
   int a2[] = {6, 8};
   sv.add_segment_range(a1, a1 + 2);
   sv.add_segment_range(a2, a2 + 2);

   boost::container::vector<int> out(4, 0);
   boost::container::vector<int>::iterator result =
      segmented_copy_if(sv.begin(), sv.end(), out.begin(), is_even());

   BOOST_TEST(result == out.end());
   BOOST_TEST_EQ(out[0], 2);
   BOOST_TEST_EQ(out[1], 4);
   BOOST_TEST_EQ(out[2], 6);
   BOOST_TEST_EQ(out[3], 8);
}

void test_copy_if_empty()
{
   test_detail::seg_vector<int> sv;
   boost::container::vector<int> out;
   boost::container::vector<int>::iterator result =
      segmented_copy_if(sv.begin(), sv.end(), out.begin(), is_even());
   BOOST_TEST(result == out.begin());
}

void test_copy_if_non_segmented()
{
   boost::container::vector<int> in;
   in.push_back(1); in.push_back(2); in.push_back(3);
   in.push_back(4); in.push_back(5);

   boost::container::vector<int> out(5, 0);
   boost::container::vector<int>::iterator result =
      segmented_copy_if(in.begin(), in.end(), out.begin(), is_even());

   std::size_t count = static_cast<std::size_t>(result - out.begin());
   BOOST_TEST_EQ(count, 2u);
   BOOST_TEST_EQ(out[0], 2);
   BOOST_TEST_EQ(out[1], 4);
}

void test_copy_if_sentinel_segmented()
{
   test_detail::seg_vector<int> sv;
   int a1[] = {1, 2, 3};
   int a2[] = {4, 5};
   int a3[] = {6, 7, 8, 9};
   sv.add_segment_range(a1, a1 + 3);
   sv.add_segment_range(a2, a2 + 2);
   sv.add_segment_range(a3, a3 + 4);

   boost::container::vector<int> out(9, 0);
   boost::container::vector<int>::iterator result =
      segmented_copy_if(sv.begin(), test_detail::make_sentinel(sv.end()), out.begin(), is_even());

   std::size_t count = static_cast<std::size_t>(result - out.begin());
   BOOST_TEST_EQ(count, 4u);
   BOOST_TEST_EQ(out[0], 2);
   BOOST_TEST_EQ(out[1], 4);
   BOOST_TEST_EQ(out[2], 6);
   BOOST_TEST_EQ(out[3], 8);
}

void test_copy_if_sentinel_non_segmented()
{
   boost::container::vector<int> in;
   in.push_back(1); in.push_back(2); in.push_back(3);
   in.push_back(4); in.push_back(5);

   boost::container::vector<int> out(5, 0);
   boost::container::vector<int>::iterator result =
      segmented_copy_if(in.begin(), test_detail::make_sentinel(in.end()), out.begin(), is_even());

   std::size_t count = static_cast<std::size_t>(result - out.begin());
   BOOST_TEST_EQ(count, 2u);
   BOOST_TEST_EQ(out[0], 2);
   BOOST_TEST_EQ(out[1], 4);
}

void test_copy_if_seg2()
{
   test_detail::seg2_vector<int> sv2;
   int a1[] = {1, 2, 3};
   int a2[] = {4, 5};
   int a3[] = {6, 7, 8, 9};
   sv2.add_flat_segment_range(a1, a1 + 3);
   sv2.add_flat_segment_range(a2, a2 + 2);
   sv2.add_flat_segment_range(a3, a3 + 4);

   boost::container::vector<int> out(9, 0);
   boost::container::vector<int>::iterator result =
      segmented_copy_if(sv2.begin(), sv2.end(), out.begin(), is_even());

   std::size_t count = static_cast<std::size_t>(result - out.begin());
   BOOST_TEST_EQ(count, 4u);
   BOOST_TEST_EQ(out[0], 2);
   BOOST_TEST_EQ(out[1], 4);
   BOOST_TEST_EQ(out[2], 6);
   BOOST_TEST_EQ(out[3], 8);
}

void test_copy_if_segmented_output()
{
   test_detail::seg_vector<int> sv;
   int a1[] = {1, 2, 3};
   int a2[] = {4, 5};
   int a3[] = {6, 7, 8, 9};
   sv.add_segment_range(a1, a1 + 3);
   sv.add_segment_range(a2, a2 + 2);
   sv.add_segment_range(a3, a3 + 4);

   test_detail::seg_vector<int> out;
   out.add_segment(3, 0);
   out.add_segment(2, 0);

   typedef test_detail::seg_vector<int>::iterator iter_t;
   iter_t result = segmented_copy_if(sv.begin(), sv.end(), out.begin(), is_even());

   std::size_t count = 0;
   iter_t it = out.begin();
   for(; it != result; ++it)
      ++count;
   BOOST_TEST_EQ(count, 4u);

   it = out.begin();
   BOOST_TEST_EQ(*it, 2); ++it;
   BOOST_TEST_EQ(*it, 4); ++it;
   BOOST_TEST_EQ(*it, 6); ++it;
   BOOST_TEST_EQ(*it, 8);
}

void test_copy_if_seg2_to_seg2()
{
   test_detail::seg2_vector<int> sv2;
   int a1[] = {1, 2, 3};
   int a2[] = {4, 5};
   int a3[] = {6, 7, 8, 9};
   sv2.add_flat_segment_range(a1, a1 + 3);
   sv2.add_flat_segment_range(a2, a2 + 2);
   sv2.add_flat_segment_range(a3, a3 + 4);

   test_detail::seg2_vector<int> out;
   {
      test_detail::seg_vector<int> s1; s1.add_segment(3, 0);
      test_detail::seg_vector<int> s2; s2.add_segment(2, 0);
      out.add_segment(s1);
      out.add_segment(s2);
   }

   typedef test_detail::seg2_vector<int>::iterator iter_t;
   iter_t result = segmented_copy_if(sv2.begin(), sv2.end(), out.begin(), is_even());

   std::size_t count = 0;
   iter_t it = out.begin();
   for(; it != result; ++it)
      ++count;
   BOOST_TEST_EQ(count, 4u);

   it = out.begin();
   BOOST_TEST_EQ(*it, 2); ++it;
   BOOST_TEST_EQ(*it, 4); ++it;
   BOOST_TEST_EQ(*it, 6); ++it;
   BOOST_TEST_EQ(*it, 8);
}

void test_copy_if_seg_to_seg_misaligned()
{
   test_detail::seg_vector<int> sv;
   int a1[] = {1, 2, 3, 4, 5};
   int a2[] = {6, 7, 8};
   sv.add_segment_range(a1, a1 + 5);
   sv.add_segment_range(a2, a2 + 3);

   test_detail::seg_vector<int> out;
   out.add_segment(2, 0);
   out.add_segment(3, 0);

   typedef test_detail::seg_vector<int>::iterator iter_t;
   iter_t result = segmented_copy_if(sv.begin(), sv.end(), out.begin(), is_even());

   std::size_t count = 0;
   iter_t it = out.begin();
   for(; it != result; ++it)
      ++count;
   BOOST_TEST_EQ(count, 4u);

   it = out.begin();
   BOOST_TEST_EQ(*it, 2); ++it;
   BOOST_TEST_EQ(*it, 4); ++it;
   BOOST_TEST_EQ(*it, 6); ++it;
   BOOST_TEST_EQ(*it, 8);
}

void test_copy_if_single_segment_whole()
{
   test_detail::seg_vector<int> sv;
   int a[] = {1, 2, 3, 4, 5, 6};
   sv.add_segment_range(a, a + 6);

   boost::container::vector<int> out(6, 0);
   boost::container::vector<int>::iterator result =
      segmented_copy_if(sv.begin(), sv.end(), out.begin(), is_even());

   BOOST_TEST_EQ(static_cast<std::size_t>(result - out.begin()), 3u);
   BOOST_TEST_EQ(out[0], 2);
   BOOST_TEST_EQ(out[1], 4);
   BOOST_TEST_EQ(out[2], 6);
   BOOST_TEST_EQ(out[3], 0);
}

void test_copy_if_single_segment_interior()
{
   test_detail::seg_vector<int> sv;
   int a[] = {1, 2, 3, 4, 5, 6};
   sv.add_segment_range(a, a + 6);

   boost::container::vector<int> out(6, 0);
   boost::container::vector<int>::iterator result =
      segmented_copy_if(test_detail::iter_at(sv, 1), test_detail::iter_at(sv, 5), out.begin(), is_even());

   BOOST_TEST_EQ(static_cast<std::size_t>(result - out.begin()), 2u);
   BOOST_TEST_EQ(out[0], 2);
   BOOST_TEST_EQ(out[1], 4);
   BOOST_TEST_EQ(out[2], 0);
   BOOST_TEST_EQ(out[3], 0);
}

void test_copy_if_single_segment_empty_mid()
{
   test_detail::seg_vector<int> sv;
   int a[] = {1, 2, 3, 4, 5, 6};
   sv.add_segment_range(a, a + 6);

   test_detail::seg_vector<int>::iterator mid = test_detail::iter_at(sv, 3);
   boost::container::vector<int> out(3, 99);
   boost::container::vector<int>::iterator result = segmented_copy_if(mid, mid, out.begin(), is_even());

   BOOST_TEST(result == out.begin());
   BOOST_TEST_EQ(out[0], 99);
   BOOST_TEST_EQ(out[1], 99);
   BOOST_TEST_EQ(out[2], 99);
}

void test_copy_if_single_segment_sentinel()
{
   test_detail::seg_vector<int> sv;
   int a[] = {1, 2, 3, 4, 5, 6};
   sv.add_segment_range(a, a + 6);

   boost::container::vector<int> out(6, 0);
   boost::container::vector<int>::iterator result =
      segmented_copy_if(test_detail::iter_at(sv, 1),
                        test_detail::make_sentinel(test_detail::iter_at(sv, 5)), out.begin(), is_even());

   BOOST_TEST_EQ(static_cast<std::size_t>(result - out.begin()), 2u);
   BOOST_TEST_EQ(out[0], 2);
   BOOST_TEST_EQ(out[1], 4);
   BOOST_TEST_EQ(out[2], 0);
}

void test_copy_if_single_segment_seg2_outer()
{
   test_detail::seg2_vector<int> sv2;
   test_detail::seg_vector<int> inner;
   int a1[] = {1, 2, 3};
   int a2[] = {4, 5};
   int a3[] = {6, 7};
   inner.add_segment_range(a1, a1 + 3);
   inner.add_segment_range(a2, a2 + 2);
   inner.add_segment_range(a3, a3 + 2);
   sv2.add_segment(inner);

   boost::container::vector<int> out(8, 0);
   boost::container::vector<int>::iterator result =
      segmented_copy_if(test_detail::iter_at(sv2, 1), test_detail::iter_at(sv2, 6), out.begin(), is_even());

   BOOST_TEST_EQ(static_cast<std::size_t>(result - out.begin()), 3u);
   BOOST_TEST_EQ(out[0], 2);
   BOOST_TEST_EQ(out[1], 4);
   BOOST_TEST_EQ(out[2], 6);
   BOOST_TEST_EQ(out[3], 0);
}

void test_copy_if_single_segment_seg2_both()
{
   test_detail::seg2_vector<int> sv2;
   test_detail::seg_vector<int> inner;
   int a[] = {1, 2, 3, 4, 5, 6};
   inner.add_segment_range(a, a + 6);
   sv2.add_segment(inner);

   boost::container::vector<int> out(6, 0);
   boost::container::vector<int>::iterator result =
      segmented_copy_if(test_detail::iter_at(sv2, 1), test_detail::iter_at(sv2, 5), out.begin(), is_even());

   BOOST_TEST_EQ(static_cast<std::size_t>(result - out.begin()), 2u);
   BOOST_TEST_EQ(out[0], 2);
   BOOST_TEST_EQ(out[1], 4);
   BOOST_TEST_EQ(out[2], 0);
}

void test_copy_if_single_segment_src_multi_dst()
{
   test_detail::seg_vector<int> sv;
   int a[] = {1, 2, 3, 4, 5, 6, 7, 8};
   sv.add_segment_range(a, a + 8);

   test_detail::seg_vector<int> out;
   out.add_segment(2, 0);
   out.add_segment(3, 0);

   typedef test_detail::seg_vector<int>::iterator iter_t;
   iter_t result = segmented_copy_if(test_detail::iter_at(sv, 1), test_detail::iter_at(sv, 7), out.begin(), is_even());

   BOOST_TEST(result == test_detail::iter_at(out, 3));
   iter_t it = out.begin();
   BOOST_TEST_EQ(*it, 2); ++it;
   BOOST_TEST_EQ(*it, 4); ++it;
   BOOST_TEST_EQ(*it, 6); ++it;
   BOOST_TEST_EQ(*it, 0);
}

void test_copy_if_single_segment_src_and_dst()
{
   test_detail::seg_vector<int> sv;
   int a[] = {1, 2, 3, 4, 5, 6};
   sv.add_segment_range(a, a + 6);

   test_detail::seg_vector<int> out;
   out.add_segment(4, 0);

   typedef test_detail::seg_vector<int>::iterator iter_t;
   iter_t result = segmented_copy_if(test_detail::iter_at(sv, 1), test_detail::iter_at(sv, 5), out.begin(), is_even());

   BOOST_TEST(result == test_detail::iter_at(out, 2));
   iter_t it = out.begin();
   BOOST_TEST_EQ(*it, 2); ++it;
   BOOST_TEST_EQ(*it, 4); ++it;
   BOOST_TEST_EQ(*it, 0); ++it;
   BOOST_TEST_EQ(*it, 0);
}

void test_copy_if_single_segment_dst_from_flat()
{
   boost::container::vector<int> in;
   in.push_back(1); in.push_back(2); in.push_back(3); in.push_back(4); in.push_back(5);

   test_detail::seg_vector<int> out;
   out.add_segment(4, 0);

   typedef test_detail::seg_vector<int>::iterator iter_t;
   iter_t result = segmented_copy_if(in.begin(), in.end(), out.begin(), is_even());

   BOOST_TEST(result == test_detail::iter_at(out, 2));
   iter_t it = out.begin();
   BOOST_TEST_EQ(*it, 2); ++it;
   BOOST_TEST_EQ(*it, 4); ++it;
   BOOST_TEST_EQ(*it, 0); ++it;
   BOOST_TEST_EQ(*it, 0);
}

//////////////////////////////////////////////////////////////////////////////
// Shape matrix.
//
// for_each_shape2_all walks the twelve branch specs of the source range
// against the twelve of the destination range, so the two segmentations vary
// independently: 144 pairs per size pair, over both segmentation depths.  The
// 'e' specs carry empty segments, which is the only way to reach the
// sfirst == slast branch of segmented_copy_if_dst_bounded.
//
// The output length of copy_if is data dependent, so the destination is run
// twice per source size: once sized to exactly the expected number of matches,
// which puts the guard immediately after the last slot the algorithm may
// write, and once with three spare slots, which pins the slots it must leave
// alone.  The guard value is even, so a copy_if that read one past the end of
// the source would copy the guard and be caught by the first form.
//
// The reference answer is filtered from flatten_n_ints over the logical source
// range; flatten_all_ints is not used, as it deliberately includes the guard.
//////////////////////////////////////////////////////////////////////////////

const int shape_filler = -998;   // guard just past the end of every range; even
const int shape_fill   = -1;     // every destination slot before the call

const int shape_dst_vals[] = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1};

std::size_t shape_match_count(const int* v, std::size_t n)
{
   std::size_t c = 0;
   for(std::size_t i = 0; i != n; ++i)
      if(is_even()(v[i]))
         ++c;
   return c;
}

struct copy_if_shape_check
{
   template<class CSrc, class CDst>
   void operator()(CSrc& src, std::size_t n_src, const char* src_spec,
                   CDst& dst, std::size_t n_dst, const char* dst_spec) const
   {
      typedef typename CDst::iterator dst_iter_t;

      const boost::container::vector<int> in = test_detail::flatten_n_ints(src, n_src);
      boost::container::vector<int> ref;
      for(std::size_t i = 0; i != n_src; ++i)
         if(is_even()(in[i]))
            ref.push_back(in[i]);

      const dst_iter_t r = segmented_copy_if
         (src.begin(), test_detail::iter_at(src, n_src), dst.begin(), is_even());

      BOOST_TEST(r == test_detail::iter_at(dst, ref.size()));
      BOOST_TEST(test_detail::filler_intact(src, n_src, shape_filler));
      BOOST_TEST(test_detail::filler_intact(dst, n_dst, shape_filler));

      const boost::container::vector<int> got = test_detail::flatten_n_ints(dst, n_dst);
      BOOST_TEST_EQ(got.size(), n_dst);
      for(std::size_t i = 0; i != n_dst; ++i)
         BOOST_TEST_EQ(got[i], i < ref.size() ? ref[i] : shape_fill);

      // The source is read-only, so it has to come back unchanged.
      const boost::container::vector<int> src_after = test_detail::flatten_n_ints(src, n_src);
      for(std::size_t i = 0; i != n_src; ++i)
         BOOST_TEST_EQ(src_after[i], in[i]);

      BOOST_TEST(src_spec != 0 && dst_spec != 0);
   }
};

void test_copy_if_shape_matrix()
{
   // Alternating, every element matching and no element matching, so that the
   // all-match and no-match answers are covered by the matrix as well.
   static const int mixed[]  = {2, 7, 4, 9, 6, 11, 8, 13, 10, 15};
   static const int all_hit[] = {2, 4, 6, 8, 10, 12};
   static const int no_hit[]  = {1, 3, 5, 7, 9, 11};
   static const int* const sets[] = {mixed, all_hit, no_hit};
   static const std::size_t set_len[] = {10u, 6u, 6u};
   static const std::size_t sizes[] = {0u, 1u, 2u, 3u, 5u, 6u};

   for(std::size_t s = 0; s != sizeof(sets)/sizeof(sets[0]); ++s) {
      for(std::size_t i = 0; i != sizeof(sizes)/sizeof(sizes[0]); ++i) {
         const std::size_t n_src = sizes[i];
         if(n_src > set_len[s])
            continue;
         const std::size_t m = shape_match_count(sets[s], n_src);
         test_detail::for_each_shape2_all<int, int>
            (sets[s], n_src, shape_dst_vals, m, shape_filler, copy_if_shape_check());
         test_detail::for_each_shape2_all<int, int>
            (sets[s], n_src, shape_dst_vals, m + 3u, shape_filler, copy_if_shape_check());
      }
   }
}

//////////////////////////////////////////////////////////////////////////////
// Unrolled cleanup block coverage.
//
// copy_if_cleanup_blocks<32> only runs with a random-access source and a
// destination segment that still has room for 32 more elements.  Every other
// range in this file is far shorter than one block, so without these cases the
// block and its remainder never execute and all the work falls to the checked
// tail loop.
//
// The destination is sized to the exact match count, so with the alternating
// data (half the elements match) a destination segment only has room for a
// block once the source is around 64 long; the all_match data reaches the
// block at 33.  Both are used, which makes the block stop on the source count
// in some cases and on the destination room in others, and leaves the tail
// loop both an empty and a non-empty remainder.
//////////////////////////////////////////////////////////////////////////////

template<class CSrc, class CDst>
void copy_if_block_case(const char* src_spec, const char* dst_spec, std::size_t n, bool all_match)
{
   boost::container::vector<int> vals;
   vals.reserve(n);
   for(std::size_t i = 0; i != n; ++i)
      vals.push_back(all_match ? 2*static_cast<int>(i) + 2 : static_cast<int>(i) + 1);

   CSrc src;
   test_detail::make_range(src, src_spec, &vals[0], n, shape_filler);

   boost::container::vector<int> ref;
   for(std::size_t i = 0; i != n; ++i)
      if(is_even()(vals[i]))
         ref.push_back(vals[i]);

   CDst dst;
   test_detail::make_dest_range(dst, dst_spec, ref.size(), shape_fill, shape_filler);

   typedef typename CDst::iterator dst_iter_t;
   const dst_iter_t r = segmented_copy_if
      (src.begin(), test_detail::iter_at(src, n), dst.begin(), is_even());

   BOOST_TEST(r == test_detail::iter_at(dst, ref.size()));
   BOOST_TEST(test_detail::filler_intact(src, n, shape_filler));
   BOOST_TEST(test_detail::filler_intact(dst, ref.size(), shape_filler));

   const boost::container::vector<int> got = test_detail::flatten_n_ints(dst, ref.size());
   BOOST_TEST_EQ(got.size(), ref.size());
   for(std::size_t i = 0; i != got.size(); ++i)
      BOOST_TEST_EQ(got[i], ref[i]);
}

void test_copy_if_unrolled_blocks()
{
   typedef test_detail::seg_vector<int>  s1_t;
   typedef test_detail::seg2_vector<int> s2_t;

   static const std::size_t sizes[] = {33u, 39u, 64u, 200u, 320u};

   for(std::size_t i = 0; i != sizeof(sizes)/sizeof(sizes[0]); ++i) {
      const std::size_t n = sizes[i];
      for(int am = 0; am != 2; ++am) {
         const bool a = am != 0;
         copy_if_block_case<s1_t, s1_t>("s",  "s",  n, a);
         copy_if_block_case<s1_t, s1_t>("m",  "m",  n, a);
         copy_if_block_case<s1_t, s1_t>("e",  "e",  n, a);
         copy_if_block_case<s1_t, s2_t>("s",  "ss", n, a);
         copy_if_block_case<s1_t, s2_t>("m",  "mm", n, a);
         copy_if_block_case<s2_t, s1_t>("sm", "s",  n, a);
         copy_if_block_case<s2_t, s2_t>("mm", "mm", n, a);
         copy_if_block_case<s2_t, s2_t>("ee", "ee", n, a);
      }
   }
}

int main()
{
   test_copy_if_full_range();
   test_copy_if_none_match();
   test_copy_if_all_match();
   test_copy_if_single_segment_whole();
   test_copy_if_single_segment_interior();
   test_copy_if_single_segment_empty_mid();
   test_copy_if_single_segment_sentinel();
   test_copy_if_single_segment_seg2_outer();
   test_copy_if_single_segment_seg2_both();
   test_copy_if_empty();
   test_copy_if_non_segmented();
   test_copy_if_sentinel_segmented();
   test_copy_if_sentinel_non_segmented();
   test_copy_if_seg2();
   test_copy_if_segmented_output();
   test_copy_if_seg2_to_seg2();
   test_copy_if_seg_to_seg_misaligned();
   test_copy_if_single_segment_src_multi_dst();
   test_copy_if_single_segment_src_and_dst();
   test_copy_if_single_segment_dst_from_flat();
   test_copy_if_shape_matrix();
   test_copy_if_unrolled_blocks();
   return boost::report_errors();
}
