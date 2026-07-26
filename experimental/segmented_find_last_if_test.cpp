//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2025-2026. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////

#include <boost/container/experimental/segmented_find_last_if.hpp>
#include <boost/core/lightweight_test.hpp>
#include "segmented_test_helper.hpp"
#include <boost/container/vector.hpp>

using namespace boost::container;

struct is_negative
{
   bool operator()(int x) const { return x < 0; }
};

struct equals_val
{
   int v;
   equals_val(int x) : v(x) {}
   bool operator()(int x) const { return x == v; }
};

void test_find_last_if_present()
{
   test_detail::seg_vector<int> sv;
   int a1[] = {1, -2, 3};
   int a2[] = {-4, 5, -6};
   sv.add_segment_range(a1, a1 + 3);
   sv.add_segment_range(a2, a2 + 3);

   test_detail::seg_vector<int>::iterator it =
      segmented_find_last_if(sv.begin(), sv.end(), is_negative());
   BOOST_TEST(it != sv.end());
   BOOST_TEST_EQ(*it, -6);
}

void test_find_last_if_present_first_segment()
{
   test_detail::seg_vector<int> sv;
   int a1[] = {1, -2, 3};
   int a2[] = {4, 5, 6};
   sv.add_segment_range(a1, a1 + 3);
   sv.add_segment_range(a2, a2 + 3);

   test_detail::seg_vector<int>::iterator it =
      segmented_find_last_if(sv.begin(), sv.end(), is_negative());
   BOOST_TEST(it != sv.end());
   BOOST_TEST_EQ(*it, -2);
}

void test_find_last_if_empty()
{
   test_detail::seg_vector<int> sv;
   test_detail::seg_vector<int>::iterator it =
      segmented_find_last_if(sv.begin(), sv.end(), is_negative());
   BOOST_TEST(it == sv.end());
}

void test_find_last_if_non_segmented()
{
   boost::container::vector<int> v;
   v.push_back(-1);
   v.push_back(-2);
   v.push_back(3);

   boost::container::vector<int>::iterator it =
      segmented_find_last_if(v.begin(), v.end(), is_negative());
   BOOST_TEST(it != v.end());
   BOOST_TEST_EQ(*it, -2);

   v.clear();
   v.push_back(1);
   v.push_back(2);
   v.push_back(3);
   it = segmented_find_last_if(v.begin(), v.end(), is_negative());
   BOOST_TEST(it == v.end());
}

void test_find_last_if_sentinel_segmented()
{
   test_detail::seg_vector<int> sv;
   int a1[] = {1, -2, 3};
   int a2[] = {-4, 5, -6};
   sv.add_segment_range(a1, a1 + 3);
   sv.add_segment_range(a2, a2 + 3);

   test_detail::seg_vector<int>::iterator it =
      segmented_find_last_if(sv.begin(), test_detail::make_sentinel(sv.end()), is_negative());
   BOOST_TEST(it != sv.end());
   BOOST_TEST_EQ(*it, -6);
}

void test_find_last_if_sentinel_non_segmented()
{
   boost::container::vector<int> v;
   v.push_back(-1);
   v.push_back(-2);
   v.push_back(3);

   boost::container::vector<int>::iterator it =
      segmented_find_last_if(v.begin(), test_detail::make_sentinel(v.end()), is_negative());
   BOOST_TEST(it != v.end());
   BOOST_TEST_EQ(*it, -2);
}

void test_find_last_if_seg2()
{
   test_detail::seg2_vector<int> sv2;
   int a1[] = {1, -2, 3};
   int a2[] = {-4, 5, -6};
   sv2.add_flat_segment_range(a1, a1 + 3);
   sv2.add_flat_segment_range(a2, a2 + 3);

   test_detail::seg2_vector<int>::iterator it =
      segmented_find_last_if(sv2.begin(), sv2.end(), is_negative());
   BOOST_TEST(it != sv2.end());
   BOOST_TEST_EQ(*it, -6);
}

void test_find_last_if_every_position()
{
   test_detail::seg_vector<int> sv;
   int a1[] = {10, 20, 30};
   int a2[] = {40, 50};
   int a3[] = {60, 70, 80, 90};
   sv.add_segment_range(a1, a1 + 3);
   sv.add_segment_range(a2, a2 + 2);
   sv.add_segment_range(a3, a3 + 4);

   int vals[] = {10, 20, 30, 40, 50, 60, 70, 80, 90};
   const int N = 9;
   typedef test_detail::seg_vector<int>::iterator iter_t;

   iter_t expected = sv.begin();
   for(int i = 0; i < N; ++i, ++expected) {
      iter_t it = segmented_find_last_if(sv.begin(), sv.end(), equals_val(vals[i]));
      BOOST_TEST(it != sv.end());
      BOOST_TEST_EQ(*it, vals[i]);
      BOOST_TEST(it == expected);
   }
   BOOST_TEST(segmented_find_last_if(sv.begin(), sv.end(), equals_val(999)) == sv.end());
}

void test_find_last_if_every_position_seg2()
{
   test_detail::seg2_vector<int> sv2;
   int a1[] = {10, 20, 30};
   int a2[] = {40, 50};
   int a3[] = {60, 70, 80, 90};
   sv2.add_flat_segment_range(a1, a1 + 3);
   sv2.add_flat_segment_range(a2, a2 + 2);
   sv2.add_flat_segment_range(a3, a3 + 4);

   int vals[] = {10, 20, 30, 40, 50, 60, 70, 80, 90};
   const int N = 9;
   typedef test_detail::seg2_vector<int>::iterator iter_t;

   iter_t expected = sv2.begin();
   for(int i = 0; i < N; ++i, ++expected) {
      iter_t it = segmented_find_last_if(sv2.begin(), sv2.end(), equals_val(vals[i]));
      BOOST_TEST(it != sv2.end());
      BOOST_TEST_EQ(*it, vals[i]);
      BOOST_TEST(it == expected);
   }
   BOOST_TEST(segmented_find_last_if(sv2.begin(), sv2.end(), equals_val(999)) == sv2.end());
}

// Runs segmented_find_last_if over a sub-range whose segmentation shape is
// dictated by a branch spec, so that every level of the recursive dispatch is
// exercised on its single-segment path, on its multi-segment path and on the
// multi-segment path with empty segments interleaved.
template<class Pred>
struct find_last_if_shape_check
{
   Pred pred;

   explicit find_last_if_shape_check(Pred p) : pred(p) {}

   template<class Cont>
   void operator()(Cont& c, std::size_t n, const char* spec) const
   {
      typedef typename Cont::iterator iter_t;
      const iter_t first = c.begin();
      const iter_t last  = test_detail::iter_at(c, n);

      // Reference: naive backward scan over a flattened copy of the range.
      const boost::container::vector<int> flat = test_detail::flatten_ints(first, last);
      std::size_t expected = flat.size();
      for(std::size_t i = flat.size(); i != 0u; --i) {
         if(pred(flat[i - 1u])) { expected = i - 1u; break; }
      }

      const iter_t r = segmented_find_last_if(first, last, pred);
      BOOST_TEST(r == test_detail::iter_at(c, expected));
      BOOST_TEST(spec != 0);
   }
};

template<class Pred>
find_last_if_shape_check<Pred> make_find_last_if_check(Pred p)
{  return find_last_if_shape_check<Pred>(p);   }

template<class Pred>
void run_find_last_if_shapes(const int* vals, std::size_t n, Pred pred)
{
   test_detail::for_each_shape_all<int>(vals, n, -999, make_find_last_if_check(pred));
   //Forward iterators reach a separate segmented implementation.
   test_detail::for_each_shape_all_fwd<int>(vals, n, -999, make_find_last_if_check(pred));
}

void test_find_last_if_shape_matrix()
{
   const std::size_t sizes[] = { 0u, 1u, 2u, 5u, 12u };
   for(std::size_t s = 0; s != sizeof(sizes)/sizeof(sizes[0]); ++s) {
      const std::size_t n = sizes[s];

      {  //Every value appears twice, so the last match and the first match
         //differ and a result carried across a segment boundary is observable.
         int vals[16];
         for(int i = 0; i != 16; ++i)
            vals[i] = i/2 + 1;
         for(std::size_t v = 0; v <= n + 1u; ++v) {
            const int target = (v <= n) ? int(v) : -999;
            run_find_last_if_shapes(vals, n, equals_val(target));
         }
      }
      {  //Exactly one element satisfies the predicate, at each position in
         //turn: whenever it is not in the final segment the result has to
         //survive the segments walked after it.
         for(std::size_t p = 0; p != n; ++p) {
            int vals[16];
            for(std::size_t i = 0; i != 16u; ++i)
               vals[i] = (i == p) ? -int(i + 1u) : int(i + 1u);
            run_find_last_if_shapes(vals, n, is_negative());
         }
      }
      {  //No element satisfies the predicate, and the out-of-range filler
         //does, so an overrun would report a match.
         int vals[16];
         for(int i = 0; i != 16; ++i)
            vals[i] = i + 1;
         run_find_last_if_shapes(vals, n, is_negative());
      }
   }
}

void test_find_last_if_single_segment_sentinel()
{
   test_detail::seg_vector<int> sv;
   int a[] = {-9, 1, -2, 3, -4, 5, -9};
   sv.add_segment_range(a, a + 7);

   typedef test_detail::seg_vector<int>::iterator iter_t;
   const iter_t first = test_detail::iter_at(sv, 1);
   const iter_t last  = test_detail::iter_at(sv, 6);

   BOOST_TEST(segmented_find_last_if(first, test_detail::make_sentinel(last), is_negative())
              == test_detail::iter_at(sv, 4));
   BOOST_TEST(segmented_find_last_if(first, test_detail::make_sentinel(last), equals_val(-9)) == last);
}

// The shape matrix does cover "a match exists in an earlier segment and must
// survive a miss in the last one", but only implicitly, as one point of a
// value sweep.  This case is the one proven to catch the result-carrying bug,
// so it stays spelled out.
void test_find_last_if_forward_match_in_earlier_segment()
{
   typedef test_detail::seg_vector<int, std::forward_iterator_tag> fwd_seg_t;
   typedef fwd_seg_t::iterator iter_t;

   fwd_seg_t sv;
   int a1[] = {1, -2, 3};
   int a2[] = {4, 5, 6};
   sv.add_segment_range(a1, a1 + 3);
   sv.add_segment_range(a2, a2 + 3);

   //The last segment reached is the empty sentinel one and holds no match.
   BOOST_TEST(segmented_find_last_if(sv.begin(), sv.end(), is_negative())
              == test_detail::iter_at(sv, 1));

   //The last segment reached is a real, non-empty segment that holds no match.
   const iter_t last = test_detail::iter_at(sv, 5);
   BOOST_TEST(segmented_find_last_if(sv.begin(), last, is_negative()) == test_detail::iter_at(sv, 1));
   BOOST_TEST(segmented_find_last_if(sv.begin(), last, equals_val(6)) == last);
}

void test_find_last_if_forward_match_in_earlier_segment_seg2()
{
   typedef test_detail::seg2_vector<int, std::forward_iterator_tag> fwd_seg2_t;
   typedef fwd_seg2_t::iterator iter_t;

   fwd_seg2_t sv2;
   int a1[] = {1, -2, 3};
   int a2[] = {4, 5, 6};
   sv2.add_flat_segment_range(a1, a1 + 3);
   sv2.add_flat_segment_range(a2, a2 + 3);

   BOOST_TEST(segmented_find_last_if(sv2.begin(), sv2.end(), is_negative())
              == test_detail::iter_at(sv2, 1));

   const iter_t last = test_detail::iter_at(sv2, 5);
   BOOST_TEST(segmented_find_last_if(sv2.begin(), last, is_negative()) == test_detail::iter_at(sv2, 1));
   BOOST_TEST(segmented_find_last_if(sv2.begin(), last, equals_val(6)) == last);
}

int main()
{
   test_find_last_if_shape_matrix();
   test_find_last_if_present();
   test_find_last_if_present_first_segment();
   test_find_last_if_empty();
   test_find_last_if_non_segmented();
   test_find_last_if_sentinel_segmented();
   test_find_last_if_sentinel_non_segmented();
   test_find_last_if_seg2();
   test_find_last_if_every_position();
   test_find_last_if_every_position_seg2();
   test_find_last_if_single_segment_sentinel();
   test_find_last_if_forward_match_in_earlier_segment();
   test_find_last_if_forward_match_in_earlier_segment_seg2();
   return boost::report_errors();
}
