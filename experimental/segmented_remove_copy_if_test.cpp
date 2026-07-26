//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2025-2026. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////

#include <boost/container/experimental/segmented_remove_copy_if.hpp>
#include <boost/core/lightweight_test.hpp>
#include "segmented_test_helper.hpp"
#include <boost/container/vector.hpp>

using namespace boost::container;

struct is_even
{
   bool operator()(int x) const { return x % 2 == 0; }
};

struct is_negative
{
   bool operator()(int x) const { return x < 0; }
};

void test_remove_copy_if_segmented()
{
   test_detail::seg_vector<int> sv;
   int a1[] = {1, 2, 3};
   int a2[] = {4, 5, 6};
   sv.add_segment_range(a1, a1 + 3);
   sv.add_segment_range(a2, a2 + 3);

   boost::container::vector<int> out(6, 0);
   boost::container::vector<int>::iterator r = segmented_remove_copy_if(sv.begin(), sv.end(), out.begin(), is_even());

   int expected[] = {1, 3, 5};
   std::size_t count = static_cast<std::size_t>(r - out.begin());
   BOOST_TEST_EQ(count, 3u);
   for(std::size_t i = 0; i < count; ++i)
      BOOST_TEST_EQ(out[i], expected[i]);
}

void test_remove_copy_if_empty()
{
   test_detail::seg_vector<int> sv;
   boost::container::vector<int> out;
   boost::container::vector<int>::iterator r = segmented_remove_copy_if(sv.begin(), sv.end(), out.begin(), is_even());
   BOOST_TEST(r == out.begin());
}

void test_remove_copy_if_none_removed()
{
   test_detail::seg_vector<int> sv;
   int a[] = {1, 3, 5};
   sv.add_segment_range(a, a + 3);

   boost::container::vector<int> out(3, 0);
   boost::container::vector<int>::iterator r = segmented_remove_copy_if(sv.begin(), sv.end(), out.begin(), is_even());
   BOOST_TEST_EQ(static_cast<std::size_t>(r - out.begin()), 3u);
   BOOST_TEST_EQ(out[0], 1);
   BOOST_TEST_EQ(out[1], 3);
   BOOST_TEST_EQ(out[2], 5);
}

void test_remove_copy_if_all_removed()
{
   test_detail::seg_vector<int> sv;
   int a[] = {2, 4, 6};
   sv.add_segment_range(a, a + 3);

   boost::container::vector<int> out(3, 0);
   boost::container::vector<int>::iterator r = segmented_remove_copy_if(sv.begin(), sv.end(), out.begin(), is_even());
   BOOST_TEST_EQ(static_cast<std::size_t>(r - out.begin()), 0u);
}

void test_remove_copy_if_non_segmented()
{
   int src[] = {1, 2, 3, 4, 5};
   boost::container::vector<int> v(src, src + 5);
   boost::container::vector<int> out(5, 0);
   boost::container::vector<int>::iterator r = segmented_remove_copy_if(v.begin(), v.end(), out.begin(), is_even());

   std::size_t count = static_cast<std::size_t>(r - out.begin());
   BOOST_TEST_EQ(count, 3u);
   BOOST_TEST_EQ(out[0], 1);
   BOOST_TEST_EQ(out[1], 3);
   BOOST_TEST_EQ(out[2], 5);
}

void test_remove_copy_if_sentinel_segmented()
{
   test_detail::seg_vector<int> sv;
   int a1[] = {1, 2, 3};
   int a2[] = {4, 5, 6};
   sv.add_segment_range(a1, a1 + 3);
   sv.add_segment_range(a2, a2 + 3);

   boost::container::vector<int> out(6, 0);
   boost::container::vector<int>::iterator r = segmented_remove_copy_if(sv.begin(), test_detail::make_sentinel(sv.end()), out.begin(), is_even());

   int expected[] = {1, 3, 5};
   std::size_t count = static_cast<std::size_t>(r - out.begin());
   BOOST_TEST_EQ(count, 3u);
   for(std::size_t i = 0; i < count; ++i)
      BOOST_TEST_EQ(out[i], expected[i]);
}

void test_remove_copy_if_sentinel_non_segmented()
{
   int src[] = {1, 2, 3, 4, 5};
   boost::container::vector<int> v(src, src + 5);
   boost::container::vector<int> out(5, 0);
   boost::container::vector<int>::iterator r = segmented_remove_copy_if(v.begin(), test_detail::make_sentinel(v.end()), out.begin(), is_even());

   std::size_t count = static_cast<std::size_t>(r - out.begin());
   BOOST_TEST_EQ(count, 3u);
   BOOST_TEST_EQ(out[0], 1);
   BOOST_TEST_EQ(out[1], 3);
   BOOST_TEST_EQ(out[2], 5);
}

void test_remove_copy_if_seg2()
{
   test_detail::seg2_vector<int> sv2;
   int a1[] = {1, 2, 3};
   int a2[] = {4, 5, 6};
   int a3[] = {7, 8};
   sv2.add_flat_segment_range(a1, a1 + 3);
   sv2.add_flat_segment_range(a2, a2 + 3);
   sv2.add_flat_segment_range(a3, a3 + 2);

   boost::container::vector<int> out(8, 0);
   boost::container::vector<int>::iterator r = segmented_remove_copy_if(sv2.begin(), sv2.end(), out.begin(), is_even());

   int expected[] = {1, 3, 5, 7};
   std::size_t count = static_cast<std::size_t>(r - out.begin());
   BOOST_TEST_EQ(count, 4u);
   for(std::size_t i = 0; i < count; ++i)
      BOOST_TEST_EQ(out[i], expected[i]);
}

void test_remove_copy_if_single_segment_whole()
{
   test_detail::seg_vector<int> sv;
   int a[] = {1, 2, 3, 4, 5, 6};
   sv.add_segment_range(a, a + 6);

   boost::container::vector<int> out(6, 0);
   boost::container::vector<int>::iterator r =
      segmented_remove_copy_if(sv.begin(), sv.end(), out.begin(), is_even());

   BOOST_TEST_EQ(static_cast<std::size_t>(r - out.begin()), 3u);
   BOOST_TEST_EQ(out[0], 1);
   BOOST_TEST_EQ(out[1], 3);
   BOOST_TEST_EQ(out[2], 5);
   BOOST_TEST_EQ(out[3], 0);
}

void test_remove_copy_if_single_segment_interior()
{
   test_detail::seg_vector<int> sv;
   int a[] = {1, 2, 3, 4, 5, 6};
   sv.add_segment_range(a, a + 6);

   boost::container::vector<int> out(6, 0);
   boost::container::vector<int>::iterator r =
      segmented_remove_copy_if(test_detail::iter_at(sv, 1), test_detail::iter_at(sv, 5), out.begin(), is_even());

   BOOST_TEST_EQ(static_cast<std::size_t>(r - out.begin()), 2u);
   BOOST_TEST_EQ(out[0], 3);
   BOOST_TEST_EQ(out[1], 5);
   BOOST_TEST_EQ(out[2], 0);
   BOOST_TEST_EQ(out[3], 0);
}

void test_remove_copy_if_single_segment_negative_pred()
{
   test_detail::seg_vector<int> sv;
   int a[] = {1, -2, 3, -4, 5, 6};
   sv.add_segment_range(a, a + 6);

   boost::container::vector<int> out(6, 0);
   boost::container::vector<int>::iterator r =
      segmented_remove_copy_if(test_detail::iter_at(sv, 1), test_detail::iter_at(sv, 5), out.begin(), is_negative());

   BOOST_TEST_EQ(static_cast<std::size_t>(r - out.begin()), 2u);
   BOOST_TEST_EQ(out[0], 3);
   BOOST_TEST_EQ(out[1], 5);
   BOOST_TEST_EQ(out[2], 0);
}

void test_remove_copy_if_single_segment_empty_mid()
{
   test_detail::seg_vector<int> sv;
   int a[] = {1, 2, 3, 4, 5, 6};
   sv.add_segment_range(a, a + 6);

   test_detail::seg_vector<int>::iterator mid = test_detail::iter_at(sv, 3);
   boost::container::vector<int> out(3, 99);
   boost::container::vector<int>::iterator r = segmented_remove_copy_if(mid, mid, out.begin(), is_even());

   BOOST_TEST(r == out.begin());
   BOOST_TEST_EQ(out[0], 99);
   BOOST_TEST_EQ(out[1], 99);
   BOOST_TEST_EQ(out[2], 99);
}

void test_remove_copy_if_single_segment_sentinel()
{
   test_detail::seg_vector<int> sv;
   int a[] = {1, 2, 3, 4, 5, 6};
   sv.add_segment_range(a, a + 6);

   boost::container::vector<int> out(6, 0);
   boost::container::vector<int>::iterator r =
      segmented_remove_copy_if(test_detail::iter_at(sv, 1),
                               test_detail::make_sentinel(test_detail::iter_at(sv, 5)), out.begin(), is_even());

   BOOST_TEST_EQ(static_cast<std::size_t>(r - out.begin()), 2u);
   BOOST_TEST_EQ(out[0], 3);
   BOOST_TEST_EQ(out[1], 5);
   BOOST_TEST_EQ(out[2], 0);
}

void test_remove_copy_if_single_segment_seg2_outer()
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
   boost::container::vector<int>::iterator r =
      segmented_remove_copy_if(test_detail::iter_at(sv2, 1), test_detail::iter_at(sv2, 6), out.begin(), is_even());

   BOOST_TEST_EQ(static_cast<std::size_t>(r - out.begin()), 2u);
   BOOST_TEST_EQ(out[0], 3);
   BOOST_TEST_EQ(out[1], 5);
   BOOST_TEST_EQ(out[2], 0);
}

void test_remove_copy_if_single_segment_seg2_both()
{
   test_detail::seg2_vector<int> sv2;
   test_detail::seg_vector<int> inner;
   int a[] = {1, 2, 3, 4, 5, 6};
   inner.add_segment_range(a, a + 6);
   sv2.add_segment(inner);

   boost::container::vector<int> out(6, 0);
   boost::container::vector<int>::iterator r =
      segmented_remove_copy_if(test_detail::iter_at(sv2, 1), test_detail::iter_at(sv2, 5), out.begin(), is_even());

   BOOST_TEST_EQ(static_cast<std::size_t>(r - out.begin()), 2u);
   BOOST_TEST_EQ(out[0], 3);
   BOOST_TEST_EQ(out[1], 5);
   BOOST_TEST_EQ(out[2], 0);
}

void test_remove_copy_if_single_segment_src_multi_dst()
{
   test_detail::seg_vector<int> sv;
   int a[] = {1, 2, 3, 4, 5, 6, 7, 8};
   sv.add_segment_range(a, a + 8);

   test_detail::seg_vector<int> out;
   out.add_segment(2, 0);
   out.add_segment(3, 0);

   typedef test_detail::seg_vector<int>::iterator iter_t;
   iter_t r = segmented_remove_copy_if(test_detail::iter_at(sv, 1), test_detail::iter_at(sv, 7), out.begin(), is_even());

   BOOST_TEST(r == test_detail::iter_at(out, 3));
   iter_t it = out.begin();
   BOOST_TEST_EQ(*it, 3); ++it;
   BOOST_TEST_EQ(*it, 5); ++it;
   BOOST_TEST_EQ(*it, 7); ++it;
   BOOST_TEST_EQ(*it, 0);
}

void test_remove_copy_if_single_segment_src_and_dst()
{
   test_detail::seg_vector<int> sv;
   int a[] = {1, 2, 3, 4, 5, 6};
   sv.add_segment_range(a, a + 6);

   test_detail::seg_vector<int> out;
   out.add_segment(4, 0);

   typedef test_detail::seg_vector<int>::iterator iter_t;
   iter_t r = segmented_remove_copy_if(test_detail::iter_at(sv, 1), test_detail::iter_at(sv, 5), out.begin(), is_even());

   BOOST_TEST(r == test_detail::iter_at(out, 2));
   iter_t it = out.begin();
   BOOST_TEST_EQ(*it, 3); ++it;
   BOOST_TEST_EQ(*it, 5); ++it;
   BOOST_TEST_EQ(*it, 0); ++it;
   BOOST_TEST_EQ(*it, 0);
}

void test_remove_copy_if_single_segment_dst_from_flat()
{
   int src[] = {1, 2, 3, 4, 5};
   boost::container::vector<int> v(src, src + 5);

   test_detail::seg_vector<int> out;
   out.add_segment(5, 0);

   typedef test_detail::seg_vector<int>::iterator iter_t;
   iter_t r = segmented_remove_copy_if(v.begin(), v.end(), out.begin(), is_even());

   BOOST_TEST(r == test_detail::iter_at(out, 3));
   iter_t it = out.begin();
   BOOST_TEST_EQ(*it, 1); ++it;
   BOOST_TEST_EQ(*it, 3); ++it;
   BOOST_TEST_EQ(*it, 5); ++it;
   BOOST_TEST_EQ(*it, 0); ++it;
   BOOST_TEST_EQ(*it, 0);
}

//////////////////////////////////////////////////////////////////////////////
// Shape matrix.
//
// for_each_shape2_all crosses the twelve branch specs of the source range with
// the twelve of the destination, at both segmentation depths, so the two
// segmentations vary independently.  The 'e' specs carry empty segments, which
// is the only way into the sfirst == slast branch of the destination bounded
// helper with an empty destination segment.
//
// The output length is data dependent, so each source size is run against a
// destination sized to exactly the number of survivors, which puts the guard
// immediately after the last slot the algorithm may write, and against one
// with three spare slots, which pins the slots it must leave alone.  The guard
// value is odd, that is, one the predicate keeps, so a read one past the end
// of the source would copy the guard and be caught by the exact-size form.
//
// The reference answer is filtered out of flatten_n_ints over the logical
// source range, not flatten_all_ints, which deliberately includes the guard.
//////////////////////////////////////////////////////////////////////////////

const int shape_filler = -999;   // guard just past the end of every range; odd
const int shape_fill   = -1;     // every destination slot before the call

const int shape_dst_vals[] = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1};

std::size_t shape_survivor_count(const int* v, std::size_t n)
{
   std::size_t c = 0;
   for(std::size_t i = 0; i != n; ++i)
      if(!is_even()(v[i]))
         ++c;
   return c;
}

struct remove_copy_if_shape_check
{
   template<class CSrc, class CDst>
   void operator()(CSrc& src, std::size_t n_src, const char* src_spec,
                   CDst& dst, std::size_t n_dst, const char* dst_spec) const
   {
      typedef typename CDst::iterator dst_iter_t;

      const boost::container::vector<int> in = test_detail::flatten_n_ints(src, n_src);
      boost::container::vector<int> ref;
      for(std::size_t i = 0; i != n_src; ++i)
         if(!is_even()(in[i]))
            ref.push_back(in[i]);

      const dst_iter_t r = segmented_remove_copy_if
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

void test_remove_copy_if_shape_matrix()
{
   // Alternating, nothing removed and everything removed, so the empty and the
   // full answer are both covered by the matrix.
   static const int mixed[]  = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
   static const int keep[]   = {1, 3, 5, 7, 9, 11};
   static const int drop[]   = {2, 4, 6, 8, 10, 12};
   static const int* const sets[] = {mixed, keep, drop};
   static const std::size_t set_len[] = {10u, 6u, 6u};
   static const std::size_t sizes[] = {0u, 1u, 2u, 3u, 5u, 6u};

   for(std::size_t s = 0; s != sizeof(sets)/sizeof(sets[0]); ++s) {
      for(std::size_t i = 0; i != sizeof(sizes)/sizeof(sizes[0]); ++i) {
         const std::size_t n_src = sizes[i];
         if(n_src > set_len[s])
            continue;
         const std::size_t m = shape_survivor_count(sets[s], n_src);
         test_detail::for_each_shape2_all<int, int>
            (sets[s], n_src, shape_dst_vals, m, shape_filler, remove_copy_if_shape_check());
         test_detail::for_each_shape2_all<int, int>
            (sets[s], n_src, shape_dst_vals, m + 3u, shape_filler, remove_copy_if_shape_check());
      }
   }
}

//////////////////////////////////////////////////////////////////////////////
// Unrolled cleanup block coverage.
//
// remove_copy_if_cleanup_blocks<32> only runs with a random-access source and
// a destination segment that still has room for 32 more elements.  Every other
// range in this file is far shorter than one block, so without these cases the
// block and its remainder never execute and all the work falls to the checked
// tail loop.
//
// Two removal densities are used: alternating, where a destination sized to
// the survivor count only has block room from around 64 elements up, and
// nothing removed, which reaches the block at 33.  Between them the block
// stops on the source count in some cases and on the destination room in
// others, and the tail loop is handed both an empty and a non-empty remainder.
//////////////////////////////////////////////////////////////////////////////

template<class CSrc, class CDst>
void remove_copy_if_block_case(const char* src_spec, const char* dst_spec, std::size_t n, bool keep_all)
{
   boost::container::vector<int> vals;
   vals.reserve(n);
   for(std::size_t i = 0; i != n; ++i)
      vals.push_back(keep_all ? 2*static_cast<int>(i) + 1 : static_cast<int>(i) + 1);

   CSrc src;
   test_detail::make_range(src, src_spec, &vals[0], n, shape_filler);

   boost::container::vector<int> ref;
   for(std::size_t i = 0; i != n; ++i)
      if(!is_even()(vals[i]))
         ref.push_back(vals[i]);

   CDst dst;
   test_detail::make_dest_range(dst, dst_spec, ref.size(), shape_fill, shape_filler);

   typedef typename CDst::iterator dst_iter_t;
   const dst_iter_t r = segmented_remove_copy_if
      (src.begin(), test_detail::iter_at(src, n), dst.begin(), is_even());

   BOOST_TEST(r == test_detail::iter_at(dst, ref.size()));
   BOOST_TEST(test_detail::filler_intact(src, n, shape_filler));
   BOOST_TEST(test_detail::filler_intact(dst, ref.size(), shape_filler));

   const boost::container::vector<int> got = test_detail::flatten_n_ints(dst, ref.size());
   BOOST_TEST_EQ(got.size(), ref.size());
   for(std::size_t i = 0; i != got.size(); ++i)
      BOOST_TEST_EQ(got[i], ref[i]);
}

void test_remove_copy_if_unrolled_blocks()
{
   typedef test_detail::seg_vector<int>  s1_t;
   typedef test_detail::seg2_vector<int> s2_t;

   static const std::size_t sizes[] = {33u, 39u, 64u, 200u, 320u};

   for(std::size_t i = 0; i != sizeof(sizes)/sizeof(sizes[0]); ++i) {
      const std::size_t n = sizes[i];
      for(int ka = 0; ka != 2; ++ka) {
         const bool k = ka != 0;
         remove_copy_if_block_case<s1_t, s1_t>("s",  "s",  n, k);
         remove_copy_if_block_case<s1_t, s1_t>("m",  "m",  n, k);
         remove_copy_if_block_case<s1_t, s1_t>("e",  "e",  n, k);
         remove_copy_if_block_case<s1_t, s2_t>("s",  "ss", n, k);
         remove_copy_if_block_case<s1_t, s2_t>("m",  "mm", n, k);
         remove_copy_if_block_case<s2_t, s1_t>("sm", "s",  n, k);
         remove_copy_if_block_case<s2_t, s2_t>("mm", "mm", n, k);
         remove_copy_if_block_case<s2_t, s2_t>("ee", "ee", n, k);
      }
   }
}

int main()
{
   test_remove_copy_if_segmented();
   test_remove_copy_if_empty();
   test_remove_copy_if_none_removed();
   test_remove_copy_if_all_removed();
   test_remove_copy_if_single_segment_whole();
   test_remove_copy_if_single_segment_interior();
   test_remove_copy_if_single_segment_negative_pred();
   test_remove_copy_if_single_segment_empty_mid();
   test_remove_copy_if_single_segment_sentinel();
   test_remove_copy_if_single_segment_seg2_outer();
   test_remove_copy_if_single_segment_seg2_both();
   test_remove_copy_if_non_segmented();
   test_remove_copy_if_sentinel_segmented();
   test_remove_copy_if_sentinel_non_segmented();
   test_remove_copy_if_seg2();
   test_remove_copy_if_single_segment_src_multi_dst();
   test_remove_copy_if_single_segment_src_and_dst();
   test_remove_copy_if_single_segment_dst_from_flat();
   test_remove_copy_if_shape_matrix();
   test_remove_copy_if_unrolled_blocks();
   return boost::report_errors();
}
