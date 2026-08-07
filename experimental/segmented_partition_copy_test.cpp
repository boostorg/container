//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2025-2026. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////

#include <boost/container/experimental/segmented_partition_copy.hpp>
#include "../test/lightweight_test.hpp"
#include "segmented_test_helper.hpp"
#include <boost/container/vector.hpp>
#include <utility>

using namespace boost::container;

struct is_even
{
   bool operator()(int x) const { return x % 2 == 0; }
};

void test_partition_copy_segmented()
{
   test_detail::seg_vector<int> sv;
   int a1[] = {1, 2, 3};
   int a2[] = {4, 5, 6};
   int a3[] = {7};
   sv.add_segment_range(a1, a1 + 3);
   sv.add_segment_range(a2, a2 + 3);
   sv.add_segment_range(a3, a3 + 1);

   boost::container::vector<int> evens(7, 0);
   boost::container::vector<int> odds(7, 0);

   std::pair<boost::container::vector<int>::iterator, boost::container::vector<int>::iterator> r =
      segmented_partition_copy(sv.begin(), sv.end(), evens.begin(), odds.begin(), is_even());

   std::size_t ne = static_cast<std::size_t>(r.first  - evens.begin());
   std::size_t no = static_cast<std::size_t>(r.second - odds.begin());
   BOOST_TEST_EQ(ne, 3u);
   BOOST_TEST_EQ(no, 4u);

   BOOST_TEST_EQ(evens[0], 2);
   BOOST_TEST_EQ(evens[1], 4);
   BOOST_TEST_EQ(evens[2], 6);

   BOOST_TEST_EQ(odds[0], 1);
   BOOST_TEST_EQ(odds[1], 3);
   BOOST_TEST_EQ(odds[2], 5);
   BOOST_TEST_EQ(odds[3], 7);
}

void test_partition_copy_empty()
{
   test_detail::seg_vector<int> sv;
   boost::container::vector<int> evens;
   boost::container::vector<int> odds;
   std::pair<boost::container::vector<int>::iterator, boost::container::vector<int>::iterator> r =
      segmented_partition_copy(sv.begin(), sv.end(), evens.begin(), odds.begin(), is_even());
   BOOST_TEST(r.first  == evens.begin());
   BOOST_TEST(r.second == odds.begin());
}

void test_partition_copy_all_true()
{
   test_detail::seg_vector<int> sv;
   int a[] = {2, 4, 6};
   sv.add_segment_range(a, a + 3);

   boost::container::vector<int> evens(3, 0);
   boost::container::vector<int> odds(3, 0);
   std::pair<boost::container::vector<int>::iterator, boost::container::vector<int>::iterator> r =
      segmented_partition_copy(sv.begin(), sv.end(), evens.begin(), odds.begin(), is_even());

   BOOST_TEST_EQ(static_cast<std::size_t>(r.first  - evens.begin()), 3u);
   BOOST_TEST_EQ(static_cast<std::size_t>(r.second - odds.begin()),  0u);
}

void test_partition_copy_non_segmented()
{
   int src[] = {1, 2, 3, 4, 5};
   boost::container::vector<int> v(src, src + 5);
   boost::container::vector<int> evens(5, 0);
   boost::container::vector<int> odds(5, 0);

   std::pair<boost::container::vector<int>::iterator, boost::container::vector<int>::iterator> r =
      segmented_partition_copy(v.begin(), v.end(), evens.begin(), odds.begin(), is_even());

   std::size_t ne = static_cast<std::size_t>(r.first  - evens.begin());
   std::size_t no = static_cast<std::size_t>(r.second - odds.begin());
   BOOST_TEST_EQ(ne, 2u);
   BOOST_TEST_EQ(no, 3u);
   BOOST_TEST_EQ(evens[0], 2);
   BOOST_TEST_EQ(evens[1], 4);
   BOOST_TEST_EQ(odds[0], 1);
   BOOST_TEST_EQ(odds[1], 3);
   BOOST_TEST_EQ(odds[2], 5);
}

void test_partition_copy_sentinel_segmented()
{
   test_detail::seg_vector<int> sv;
   int a1[] = {1, 2, 3};
   int a2[] = {4, 5, 6};
   int a3[] = {7};
   sv.add_segment_range(a1, a1 + 3);
   sv.add_segment_range(a2, a2 + 3);
   sv.add_segment_range(a3, a3 + 1);

   boost::container::vector<int> evens(7, 0);
   boost::container::vector<int> odds(7, 0);

   std::pair<boost::container::vector<int>::iterator, boost::container::vector<int>::iterator> r =
      segmented_partition_copy(sv.begin(), test_detail::make_sentinel(sv.end()), evens.begin(), odds.begin(), is_even());

   std::size_t ne = static_cast<std::size_t>(r.first  - evens.begin());
   std::size_t no = static_cast<std::size_t>(r.second - odds.begin());
   BOOST_TEST_EQ(ne, 3u);
   BOOST_TEST_EQ(no, 4u);

   BOOST_TEST_EQ(evens[0], 2);
   BOOST_TEST_EQ(evens[1], 4);
   BOOST_TEST_EQ(evens[2], 6);

   BOOST_TEST_EQ(odds[0], 1);
   BOOST_TEST_EQ(odds[1], 3);
   BOOST_TEST_EQ(odds[2], 5);
   BOOST_TEST_EQ(odds[3], 7);
}

void test_partition_copy_sentinel_non_segmented()
{
   int src[] = {1, 2, 3, 4, 5};
   boost::container::vector<int> v(src, src + 5);
   boost::container::vector<int> evens(5, 0);
   boost::container::vector<int> odds(5, 0);

   std::pair<boost::container::vector<int>::iterator, boost::container::vector<int>::iterator> r =
      segmented_partition_copy(v.begin(), test_detail::make_sentinel(v.end()), evens.begin(), odds.begin(), is_even());

   std::size_t ne = static_cast<std::size_t>(r.first  - evens.begin());
   std::size_t no = static_cast<std::size_t>(r.second - odds.begin());
   BOOST_TEST_EQ(ne, 2u);
   BOOST_TEST_EQ(no, 3u);
   BOOST_TEST_EQ(evens[0], 2);
   BOOST_TEST_EQ(evens[1], 4);
   BOOST_TEST_EQ(odds[0], 1);
   BOOST_TEST_EQ(odds[1], 3);
   BOOST_TEST_EQ(odds[2], 5);
}

void test_partition_copy_seg2()
{
   test_detail::seg2_vector<int> sv2;
   int a1[] = {1, 2, 3};
   int a2[] = {4, 5, 6};
   sv2.add_flat_segment_range(a1, a1 + 3);
   sv2.add_flat_segment_range(a2, a2 + 3);

   boost::container::vector<int> evens(6, 0);
   boost::container::vector<int> odds(6, 0);

   std::pair<boost::container::vector<int>::iterator, boost::container::vector<int>::iterator> r =
      segmented_partition_copy(sv2.begin(), sv2.end(), evens.begin(), odds.begin(), is_even());

   std::size_t ne = static_cast<std::size_t>(r.first  - evens.begin());
   std::size_t no = static_cast<std::size_t>(r.second - odds.begin());
   BOOST_TEST_EQ(ne, 3u);
   BOOST_TEST_EQ(no, 3u);

   BOOST_TEST_EQ(evens[0], 2);
   BOOST_TEST_EQ(evens[1], 4);
   BOOST_TEST_EQ(evens[2], 6);

   BOOST_TEST_EQ(odds[0], 1);
   BOOST_TEST_EQ(odds[1], 3);
   BOOST_TEST_EQ(odds[2], 5);
}

void test_partition_copy_single_segment_whole()
{
   test_detail::seg_vector<int> sv;
   int a[] = {1, 2, 3, 4, 5, 6};
   sv.add_segment_range(a, a + 6);

   boost::container::vector<int> evens(6, 0);
   boost::container::vector<int> odds(6, 0);
   std::pair<boost::container::vector<int>::iterator, boost::container::vector<int>::iterator> r =
      segmented_partition_copy(sv.begin(), sv.end(), evens.begin(), odds.begin(), is_even());

   BOOST_TEST_EQ(static_cast<std::size_t>(r.first  - evens.begin()), 3u);
   BOOST_TEST_EQ(static_cast<std::size_t>(r.second - odds.begin()),  3u);
   BOOST_TEST_EQ(evens[0], 2);
   BOOST_TEST_EQ(evens[1], 4);
   BOOST_TEST_EQ(evens[2], 6);
   BOOST_TEST_EQ(odds[0], 1);
   BOOST_TEST_EQ(odds[1], 3);
   BOOST_TEST_EQ(odds[2], 5);
}

void test_partition_copy_single_segment_interior()
{
   test_detail::seg_vector<int> sv;
   int a[] = {1, 2, 3, 4, 5, 6};
   sv.add_segment_range(a, a + 6);

   boost::container::vector<int> evens(6, 0);
   boost::container::vector<int> odds(6, 0);
   std::pair<boost::container::vector<int>::iterator, boost::container::vector<int>::iterator> r =
      segmented_partition_copy(test_detail::iter_at(sv, 1), test_detail::iter_at(sv, 5),
                               evens.begin(), odds.begin(), is_even());

   BOOST_TEST_EQ(static_cast<std::size_t>(r.first  - evens.begin()), 2u);
   BOOST_TEST_EQ(static_cast<std::size_t>(r.second - odds.begin()),  2u);
   BOOST_TEST_EQ(evens[0], 2);
   BOOST_TEST_EQ(evens[1], 4);
   BOOST_TEST_EQ(evens[2], 0);
   BOOST_TEST_EQ(odds[0], 3);
   BOOST_TEST_EQ(odds[1], 5);
   BOOST_TEST_EQ(odds[2], 0);
}

void test_partition_copy_single_segment_all_true()
{
   test_detail::seg_vector<int> sv;
   int a[] = {1, 2, 4, 6, 8, 9};
   sv.add_segment_range(a, a + 6);

   boost::container::vector<int> evens(6, 0);
   boost::container::vector<int> odds(6, 0);
   std::pair<boost::container::vector<int>::iterator, boost::container::vector<int>::iterator> r =
      segmented_partition_copy(test_detail::iter_at(sv, 1), test_detail::iter_at(sv, 5),
                               evens.begin(), odds.begin(), is_even());

   BOOST_TEST_EQ(static_cast<std::size_t>(r.first  - evens.begin()), 4u);
   BOOST_TEST(r.second == odds.begin());
   BOOST_TEST_EQ(evens[0], 2);
   BOOST_TEST_EQ(evens[1], 4);
   BOOST_TEST_EQ(evens[2], 6);
   BOOST_TEST_EQ(evens[3], 8);
   BOOST_TEST_EQ(evens[4], 0);
   BOOST_TEST_EQ(odds[0], 0);
}

void test_partition_copy_single_segment_all_false()
{
   test_detail::seg_vector<int> sv;
   int a[] = {2, 1, 3, 5, 7, 4};
   sv.add_segment_range(a, a + 6);

   boost::container::vector<int> evens(6, 0);
   boost::container::vector<int> odds(6, 0);
   std::pair<boost::container::vector<int>::iterator, boost::container::vector<int>::iterator> r =
      segmented_partition_copy(test_detail::iter_at(sv, 1), test_detail::iter_at(sv, 5),
                               evens.begin(), odds.begin(), is_even());

   BOOST_TEST(r.first == evens.begin());
   BOOST_TEST_EQ(static_cast<std::size_t>(r.second - odds.begin()), 4u);
   BOOST_TEST_EQ(evens[0], 0);
   BOOST_TEST_EQ(odds[0], 1);
   BOOST_TEST_EQ(odds[1], 3);
   BOOST_TEST_EQ(odds[2], 5);
   BOOST_TEST_EQ(odds[3], 7);
   BOOST_TEST_EQ(odds[4], 0);
}

void test_partition_copy_single_segment_empty_mid()
{
   test_detail::seg_vector<int> sv;
   int a[] = {1, 2, 3, 4, 5, 6};
   sv.add_segment_range(a, a + 6);

   test_detail::seg_vector<int>::iterator mid = test_detail::iter_at(sv, 3);
   boost::container::vector<int> evens(3, 99);
   boost::container::vector<int> odds(3, 99);
   std::pair<boost::container::vector<int>::iterator, boost::container::vector<int>::iterator> r =
      segmented_partition_copy(mid, mid, evens.begin(), odds.begin(), is_even());

   BOOST_TEST(r.first  == evens.begin());
   BOOST_TEST(r.second == odds.begin());
   BOOST_TEST_EQ(evens[0], 99);
   BOOST_TEST_EQ(odds[0], 99);
}

void test_partition_copy_single_segment_sentinel()
{
   test_detail::seg_vector<int> sv;
   int a[] = {1, 2, 3, 4, 5, 6};
   sv.add_segment_range(a, a + 6);

   boost::container::vector<int> evens(6, 0);
   boost::container::vector<int> odds(6, 0);
   std::pair<boost::container::vector<int>::iterator, boost::container::vector<int>::iterator> r =
      segmented_partition_copy(test_detail::iter_at(sv, 1),
                               test_detail::make_sentinel(test_detail::iter_at(sv, 5)),
                               evens.begin(), odds.begin(), is_even());

   BOOST_TEST_EQ(static_cast<std::size_t>(r.first  - evens.begin()), 2u);
   BOOST_TEST_EQ(static_cast<std::size_t>(r.second - odds.begin()),  2u);
   BOOST_TEST_EQ(evens[0], 2);
   BOOST_TEST_EQ(evens[1], 4);
   BOOST_TEST_EQ(odds[0], 3);
   BOOST_TEST_EQ(odds[1], 5);
}

void test_partition_copy_single_segment_seg2_outer()
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

   boost::container::vector<int> evens(8, 0);
   boost::container::vector<int> odds(8, 0);
   std::pair<boost::container::vector<int>::iterator, boost::container::vector<int>::iterator> r =
      segmented_partition_copy(test_detail::iter_at(sv2, 1), test_detail::iter_at(sv2, 6),
                               evens.begin(), odds.begin(), is_even());

   BOOST_TEST_EQ(static_cast<std::size_t>(r.first  - evens.begin()), 3u);
   BOOST_TEST_EQ(static_cast<std::size_t>(r.second - odds.begin()),  2u);
   BOOST_TEST_EQ(evens[0], 2);
   BOOST_TEST_EQ(evens[1], 4);
   BOOST_TEST_EQ(evens[2], 6);
   BOOST_TEST_EQ(evens[3], 0);
   BOOST_TEST_EQ(odds[0], 3);
   BOOST_TEST_EQ(odds[1], 5);
   BOOST_TEST_EQ(odds[2], 0);
}

void test_partition_copy_single_segment_seg2_both()
{
   test_detail::seg2_vector<int> sv2;
   test_detail::seg_vector<int> inner;
   int a[] = {1, 2, 3, 4, 5, 6};
   inner.add_segment_range(a, a + 6);
   sv2.add_segment(inner);

   boost::container::vector<int> evens(6, 0);
   boost::container::vector<int> odds(6, 0);
   std::pair<boost::container::vector<int>::iterator, boost::container::vector<int>::iterator> r =
      segmented_partition_copy(test_detail::iter_at(sv2, 1), test_detail::iter_at(sv2, 5),
                               evens.begin(), odds.begin(), is_even());

   BOOST_TEST_EQ(static_cast<std::size_t>(r.first  - evens.begin()), 2u);
   BOOST_TEST_EQ(static_cast<std::size_t>(r.second - odds.begin()),  2u);
   BOOST_TEST_EQ(evens[0], 2);
   BOOST_TEST_EQ(evens[1], 4);
   BOOST_TEST_EQ(evens[2], 0);
   BOOST_TEST_EQ(odds[0], 3);
   BOOST_TEST_EQ(odds[1], 5);
   BOOST_TEST_EQ(odds[2], 0);
}

void test_partition_copy_single_segment_src_and_outs()
{
   test_detail::seg_vector<int> sv;
   int a[] = {1, 2, 3, 4, 5, 6};
   sv.add_segment_range(a, a + 6);

   test_detail::seg_vector<int> evens;
   evens.add_segment(4, 0);
   test_detail::seg_vector<int> odds;
   odds.add_segment(4, 0);

   typedef test_detail::seg_vector<int>::iterator iter_t;
   std::pair<iter_t, iter_t> r =
      segmented_partition_copy(test_detail::iter_at(sv, 1), test_detail::iter_at(sv, 5),
                               evens.begin(), odds.begin(), is_even());

   BOOST_TEST(r.first  == test_detail::iter_at(evens, 2));
   BOOST_TEST(r.second == test_detail::iter_at(odds, 2));

   iter_t it = evens.begin();
   BOOST_TEST_EQ(*it, 2); ++it;
   BOOST_TEST_EQ(*it, 4); ++it;
   BOOST_TEST_EQ(*it, 0); ++it;
   BOOST_TEST_EQ(*it, 0);

   it = odds.begin();
   BOOST_TEST_EQ(*it, 3); ++it;
   BOOST_TEST_EQ(*it, 5); ++it;
   BOOST_TEST_EQ(*it, 0); ++it;
   BOOST_TEST_EQ(*it, 0);
}

void test_partition_copy_single_segment_outs_from_flat()
{
   int src[] = {1, 2, 3, 4, 5};
   boost::container::vector<int> v(src, src + 5);

   test_detail::seg_vector<int> evens;
   evens.add_segment(4, 0);
   test_detail::seg_vector<int> odds;
   odds.add_segment(5, 0);

   typedef test_detail::seg_vector<int>::iterator iter_t;
   std::pair<iter_t, iter_t> r =
      segmented_partition_copy(v.begin(), v.end(), evens.begin(), odds.begin(), is_even());

   BOOST_TEST(r.first  == test_detail::iter_at(evens, 2));
   BOOST_TEST(r.second == test_detail::iter_at(odds, 3));

   iter_t it = evens.begin();
   BOOST_TEST_EQ(*it, 2); ++it;
   BOOST_TEST_EQ(*it, 4); ++it;
   BOOST_TEST_EQ(*it, 0); ++it;
   BOOST_TEST_EQ(*it, 0);

   it = odds.begin();
   BOOST_TEST_EQ(*it, 1); ++it;
   BOOST_TEST_EQ(*it, 3); ++it;
   BOOST_TEST_EQ(*it, 5); ++it;
   BOOST_TEST_EQ(*it, 0); ++it;
   BOOST_TEST_EQ(*it, 0);
}

//////////////////////////////////////////////////////////////////////////////
// Shape matrix.
//
// partition_copy has three ranges, so this uses for_each_shape3_all: the
// twelve branch specs of the source crossed with the twelve of each of the two
// destinations, over both segmentation depths, 1728 triples per size triple.
// The 'e' specs carry empty segments, which is the only way into the
// sfirst == slast branch of the destination bounded helpers with an empty
// destination segment.
//
// Both output lengths are data dependent, so each source is run with both
// destinations sized exactly, which puts each guard immediately after the last
// slot the algorithm may write there, and once with two spare slots on each
// side, which pins the slots it must leave alone.  Sizing exactly also gives
// the geometry the spec calls out: with a two-segment destination the matching
// elements fill an intermediate destination segment exactly.
//
// The two reference answers are filtered out of flatten_n_ints over the
// logical source range, not flatten_all_ints, which includes the guard.
//////////////////////////////////////////////////////////////////////////////

const int shape_filler = -998;   // guard just past the end of every range; even
const int shape_fill   = -1;     // every destination slot before the call

const int shape_dst_vals[] = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1};

std::size_t shape_true_count(const int* v, std::size_t n)
{
   std::size_t c = 0;
   for(std::size_t i = 0; i != n; ++i)
      if(is_even()(v[i]))
         ++c;
   return c;
}

struct partition_copy_shape_check
{
   template<class CSrc, class CTrue, class CFalse>
   void operator()(CSrc& src,  std::size_t n_src, const char* src_spec,
                   CTrue& tdst, std::size_t n_t,  const char* t_spec,
                   CFalse& fdst, std::size_t n_f, const char* f_spec) const
   {
      typedef typename CTrue::iterator  t_iter_t;
      typedef typename CFalse::iterator f_iter_t;

      const boost::container::vector<int> in = test_detail::flatten_n_ints(src, n_src);
      boost::container::vector<int> t_ref, f_ref;
      for(std::size_t i = 0; i != n_src; ++i) {
         if(is_even()(in[i]))
            t_ref.push_back(in[i]);
         else
            f_ref.push_back(in[i]);
      }

      const std::pair<t_iter_t, f_iter_t> r = segmented_partition_copy
         (src.begin(), test_detail::iter_at(src, n_src), tdst.begin(), fdst.begin(), is_even());

      BOOST_TEST(r.first  == test_detail::iter_at(tdst, t_ref.size()));
      BOOST_TEST(r.second == test_detail::iter_at(fdst, f_ref.size()));
      BOOST_TEST(test_detail::filler_intact(src,  n_src, shape_filler));
      BOOST_TEST(test_detail::filler_intact(tdst, n_t,   shape_filler));
      BOOST_TEST(test_detail::filler_intact(fdst, n_f,   shape_filler));

      const boost::container::vector<int> t_got = test_detail::flatten_n_ints(tdst, n_t);
      BOOST_TEST_EQ(t_got.size(), n_t);
      for(std::size_t i = 0; i != n_t; ++i)
         BOOST_TEST_EQ(t_got[i], i < t_ref.size() ? t_ref[i] : shape_fill);

      const boost::container::vector<int> f_got = test_detail::flatten_n_ints(fdst, n_f);
      BOOST_TEST_EQ(f_got.size(), n_f);
      for(std::size_t i = 0; i != n_f; ++i)
         BOOST_TEST_EQ(f_got[i], i < f_ref.size() ? f_ref[i] : shape_fill);

      // The source is read-only, so it has to come back unchanged.
      const boost::container::vector<int> src_after = test_detail::flatten_n_ints(src, n_src);
      for(std::size_t i = 0; i != n_src; ++i)
         BOOST_TEST_EQ(src_after[i], in[i]);

      BOOST_TEST(src_spec != 0 && t_spec != 0 && f_spec != 0);
   }
};

void test_partition_copy_shape_matrix()
{
   // Alternating, everything down the true side and everything down the false
   // side, so an empty destination is exercised against a full partner.
   static const int mixed[] = {1, 2, 3, 4, 5, 6, 7, 8};
   static const int t_only[] = {2, 4, 6, 8, 10, 12};
   static const int f_only[] = {1, 3, 5, 7, 9, 11};
   static const int* const sets[] = {mixed, t_only, f_only};
   static const std::size_t set_len[] = {8u, 6u, 6u};
   static const std::size_t sizes[] = {0u, 1u, 2u, 3u, 5u, 6u};

   for(std::size_t s = 0; s != sizeof(sets)/sizeof(sets[0]); ++s) {
      for(std::size_t i = 0; i != sizeof(sizes)/sizeof(sizes[0]); ++i) {
         const std::size_t n = sizes[i];
         if(n > set_len[s])
            continue;
         const std::size_t nt = shape_true_count(sets[s], n);
         const std::size_t nf = n - nt;
         test_detail::for_each_shape3_all<int, int, int>
            (sets[s], n, shape_dst_vals, nt, shape_dst_vals, nf,
             shape_filler, partition_copy_shape_check());
         test_detail::for_each_shape3_all<int, int, int>
            (sets[s], n, shape_dst_vals, nt + 2u, shape_dst_vals, nf + 2u,
             shape_filler, partition_copy_shape_check());
      }
   }
}

//////////////////////////////////////////////////////////////////////////////
// Unrolled cleanup block coverage.
//
// partition_copy_cleanup_blocks<32> runs while both outputs have room for a
// whole block, then <8> mops up while both still have room for eight.  Both
// need a random-access source and random-access outputs.  Every other range in
// this file is far shorter than one block, so without these cases neither
// block ever runs and all the work falls to the checked leaf loop.
//
// Three routings are used: alternating, everything true and everything false.
// The all-one-side ones give the busy output room for a block from 33 elements
// up while the idle one stays empty; the alternating one only reaches the
// 32-block from around 64 up, but reaches the 8-block below that.  Together
// they stop the blocks on the source count in some cases and on output room in
// others, and hand the leaf loop both an empty and a non-empty remainder.
//////////////////////////////////////////////////////////////////////////////

//! A multi-segment spec needs one element per level to split, so an output
//! that ends up empty falls back to the single-segment spec of the same depth.
const char* shape_spec_for(const char* spec, std::size_t n)
{
   if(test_detail::shape_feasible(spec, n))
      return spec;
   return spec[1] ? "ss" : "s";
}

template<class CSrc, class CTrue, class CFalse>
void partition_copy_block_case(const char* src_spec, const char* t_spec, const char* f_spec,
                               std::size_t n, int routing)
{
   boost::container::vector<int> vals;
   vals.reserve(n);
   for(std::size_t i = 0; i != n; ++i) {
      const int k = static_cast<int>(i);
      vals.push_back(routing == 0 ? k + 1 : (routing > 0 ? 2*k + 2 : 2*k + 1));
   }

   CSrc src;
   test_detail::make_range(src, src_spec, &vals[0], n, shape_filler);

   boost::container::vector<int> t_ref, f_ref;
   for(std::size_t i = 0; i != n; ++i) {
      if(is_even()(vals[i]))
         t_ref.push_back(vals[i]);
      else
         f_ref.push_back(vals[i]);
   }

   CTrue tdst;
   CFalse fdst;
   test_detail::make_dest_range(tdst, shape_spec_for(t_spec, t_ref.size()), t_ref.size(), shape_fill, shape_filler);
   test_detail::make_dest_range(fdst, shape_spec_for(f_spec, f_ref.size()), f_ref.size(), shape_fill, shape_filler);

   typedef typename CTrue::iterator  t_iter_t;
   typedef typename CFalse::iterator f_iter_t;
   const std::pair<t_iter_t, f_iter_t> r = segmented_partition_copy
      (src.begin(), test_detail::iter_at(src, n), tdst.begin(), fdst.begin(), is_even());

   BOOST_TEST(r.first  == test_detail::iter_at(tdst, t_ref.size()));
   BOOST_TEST(r.second == test_detail::iter_at(fdst, f_ref.size()));
   BOOST_TEST(test_detail::filler_intact(src,  n,            shape_filler));
   BOOST_TEST(test_detail::filler_intact(tdst, t_ref.size(), shape_filler));
   BOOST_TEST(test_detail::filler_intact(fdst, f_ref.size(), shape_filler));

   const boost::container::vector<int> t_got = test_detail::flatten_n_ints(tdst, t_ref.size());
   BOOST_TEST_EQ(t_got.size(), t_ref.size());
   for(std::size_t i = 0; i != t_got.size(); ++i)
      BOOST_TEST_EQ(t_got[i], t_ref[i]);

   const boost::container::vector<int> f_got = test_detail::flatten_n_ints(fdst, f_ref.size());
   BOOST_TEST_EQ(f_got.size(), f_ref.size());
   for(std::size_t i = 0; i != f_got.size(); ++i)
      BOOST_TEST_EQ(f_got[i], f_ref[i]);
}

void test_partition_copy_unrolled_blocks()
{
   typedef test_detail::seg_vector<int>  s1_t;
   typedef test_detail::seg2_vector<int> s2_t;

   static const std::size_t sizes[] = {33u, 39u, 64u, 200u, 320u};
   static const int routings[] = {0, 1, -1};

   for(std::size_t i = 0; i != sizeof(sizes)/sizeof(sizes[0]); ++i) {
      const std::size_t n = sizes[i];
      for(std::size_t k = 0; k != sizeof(routings)/sizeof(routings[0]); ++k) {
         const int rt = routings[k];
         partition_copy_block_case<s1_t, s1_t, s1_t>("s",  "s",  "s",  n, rt);
         partition_copy_block_case<s1_t, s1_t, s1_t>("m",  "m",  "m",  n, rt);
         partition_copy_block_case<s1_t, s1_t, s1_t>("e",  "e",  "e",  n, rt);
         partition_copy_block_case<s1_t, s1_t, s2_t>("s",  "s",  "ss", n, rt);
         partition_copy_block_case<s1_t, s2_t, s1_t>("m",  "mm", "m",  n, rt);
         partition_copy_block_case<s2_t, s1_t, s1_t>("sm", "s",  "s",  n, rt);
         partition_copy_block_case<s2_t, s2_t, s2_t>("mm", "mm", "mm", n, rt);
         partition_copy_block_case<s2_t, s2_t, s2_t>("ee", "ee", "ee", n, rt);
      }
   }
}

//////////////////////////////////////////////////////////////////////////////
// Predicate application count.
//
// [alg.partitions] mandates "Exactly last - first applications of pred" for
// partition_copy, whatever the segmentation of the source and of either
// output.  A leaf that applies pred, then finds one of the two destination
// segments full and returns without consuming the element it has just tested
// hands that element back to the walker, which re-applies pred to it.
//////////////////////////////////////////////////////////////////////////////

struct partition_copy_count_check
{
   template<class CSrc, class CTrue, class CFalse>
   void operator()(CSrc& src,  std::size_t n_src, const char* src_spec,
                   CTrue& tdst, std::size_t n_t,  const char* t_spec,
                   CFalse& fdst, std::size_t n_f, const char* f_spec) const
   {
      test_detail::op_counter calls;
      segmented_partition_copy(src.begin(), test_detail::iter_at(src, n_src),
                               tdst.begin(), fdst.begin(),
                               test_detail::counting_pred(calls, is_even()));

      BOOST_TEST_EQ(calls.n, n_src);
      BOOST_TEST(test_detail::filler_intact(tdst, n_t, shape_filler));
      BOOST_TEST(test_detail::filler_intact(fdst, n_f, shape_filler));
      BOOST_TEST(src_spec != 0 && t_spec != 0 && f_spec != 0);
   }
};

void partition_copy_add_block(test_detail::seg_vector<int>& c, std::size_t block)
{  c.add_segment(block, 0);   }

void partition_copy_add_block(test_detail::seg2_vector<int>& c, std::size_t block)
{
   test_detail::seg_vector<int> inner;
   inner.add_segment(block, 0);
   c.add_segment(inner);
}

// Output segments of a handful of elements each, so that boundary crossings
// dominate: both outputs cross, and the depth-2 form multiplies the crossings.
template<class CDst>
void partition_copy_count_small_dst(std::size_t n, std::size_t block)
{
   boost::container::vector<int> src;
   src.reserve(n);
   for(std::size_t i = 0; i != n; ++i)
      src.push_back(static_cast<int>(i) + 1);

   CDst tdst, fdst;
   for(std::size_t room = 0; room < n; room += block) {
      partition_copy_add_block(tdst, block);
      partition_copy_add_block(fdst, block);
   }

   test_detail::op_counter calls;
   segmented_partition_copy(src.begin(), src.end(), tdst.begin(), fdst.begin(),
                            test_detail::counting_pred(calls, is_even()));
   BOOST_TEST_EQ(calls.n, n);
}

void test_partition_copy_predicate_count()
{
   static const int mixed[]  = {1, 2, 3, 4, 5, 6, 7, 8};
   static const int t_only[] = {2, 4, 6, 8, 10, 12};
   static const int f_only[] = {1, 3, 5, 7, 9, 11};
   static const int* const sets[] = {mixed, t_only, f_only};
   static const std::size_t set_len[] = {8u, 6u, 6u};
   static const std::size_t sizes[] = {0u, 1u, 2u, 5u};

   for(std::size_t s = 0; s != sizeof(sets)/sizeof(sets[0]); ++s) {
      for(std::size_t i = 0; i != sizeof(sizes)/sizeof(sizes[0]); ++i) {
         const std::size_t n = sizes[i];
         if(n > set_len[s])
            continue;
         const std::size_t nt = shape_true_count(sets[s], n);
         const std::size_t nf = n - nt;
         test_detail::for_each_shape3_all<int, int, int>
            (sets[s], n, shape_dst_vals, nt, shape_dst_vals, nf,
             shape_filler, partition_copy_count_check());
         test_detail::for_each_shape3_all<int, int, int>
            (sets[s], n, shape_dst_vals, nt + 2u, shape_dst_vals, nf + 2u,
             shape_filler, partition_copy_count_check());
      }
   }

   static const std::size_t blocks[] = {1u, 2u, 3u, 8u, 16u};
   for(std::size_t b = 0; b != sizeof(blocks)/sizeof(blocks[0]); ++b) {
      partition_copy_count_small_dst<test_detail::seg_vector<int> >(64u, blocks[b]);
      partition_copy_count_small_dst<test_detail::seg2_vector<int> >(64u, blocks[b]);
   }
}

int main()
{
   test_partition_copy_segmented();
   test_partition_copy_empty();
   test_partition_copy_all_true();
   test_partition_copy_single_segment_whole();
   test_partition_copy_single_segment_interior();
   test_partition_copy_single_segment_all_true();
   test_partition_copy_single_segment_all_false();
   test_partition_copy_single_segment_empty_mid();
   test_partition_copy_single_segment_sentinel();
   test_partition_copy_single_segment_seg2_outer();
   test_partition_copy_single_segment_seg2_both();
   test_partition_copy_non_segmented();
   test_partition_copy_sentinel_segmented();
   test_partition_copy_sentinel_non_segmented();
   test_partition_copy_seg2();
   test_partition_copy_single_segment_src_and_outs();
   test_partition_copy_single_segment_outs_from_flat();
   test_partition_copy_shape_matrix();
   test_partition_copy_unrolled_blocks();
   test_partition_copy_predicate_count();
   return boost::report_errors();
}
