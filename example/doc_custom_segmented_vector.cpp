//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2025. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////
//[doc_custom_segmented_vector
#include <boost/container/segmented_vector.hpp>

//Make sure assertions are active
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <cassert>

int main ()
{
   using namespace boost::container;

//--------------------------------------------
//          'stored_size' option
//--------------------------------------------

   //This option specifies that a segmented_vector will use "unsigned char" as
   //the type to store capacity or size internally.
   typedef segmented_vector_options< stored_size<unsigned char> >::type size_option_t;

   //Size-optimized segmented_vector is smaller than the default one.
   typedef segmented_vector<int, new_allocator<int>, size_option_t > size_optimized_segmented_vector_t;
   assert(( sizeof(size_optimized_segmented_vector_t) < sizeof(segmented_vector<int>) ));

   //Requesting capacity for more elements than representable by "unsigned char"
   //is an error in the size optimized segmented_vector.
   bool exception_thrown = false;
   /*<-*/ 
   #ifndef BOOST_NO_EXCEPTIONS
   BOOST_CONTAINER_TRY{ size_optimized_segmented_vector_t v(256); } BOOST_CONTAINER_CATCH(...){ exception_thrown = true; } BOOST_CONTAINER_CATCH_END
   #else
   exception_thrown = true;
   #endif   //BOOST_NO_EXCEPTIONS
   /*->*/
   //=try       { size_optimized_segmented_vector_t v(256); }
   //=catch(...){ exception_thrown = true;        }

   assert(exception_thrown == true);

//--------------------------------------------
//          'block_size/segment_size' option
//--------------------------------------------

   //This option specifies the desired block size for segmented_vector
   typedef segmented_vector_options< block_size<128u> >::type block_128_option_t;

   //segment_size is an alias for block_size (an alias for block_size)
   typedef segmented_vector_options< segment_size<128u> >::type segment_128_option_t;

   //This segmented_vector will allocate blocks of 128 elements 
   typedef segmented_vector<int, void, block_128_option_t > block_128_segmented_vector_t;
   assert(block_128_segmented_vector_t::get_block_size() == 128u);

   //This segmented_vector will allocate segments of 128 elements (an alias for block_size)
   typedef segmented_vector<int, void, segment_128_option_t > segment_128_segmented_vector_t;
   assert(segment_128_segmented_vector_t::get_block_size() == 128u);

//--------------------------------------------
//          'block_bytes/segment_bytes' option
//--------------------------------------------

   //This option specifies the maximum block size for segmented_vector
   //in bytes
   typedef segmented_vector_options< block_bytes<1024u> >::type block_1024_bytes_option_t;

   //This option specifies the maximum segment size for segmented_vector
   //in bytes (an alias for block_bytes)
   typedef segmented_vector_options< segment_bytes<1024u> >::type segment_1024_bytes_option_t;

   //This segmented_vector will allocate blocks of 1024 bytes
   typedef segmented_vector<int, void, block_1024_bytes_option_t > block_1024_bytes_segmented_vector_t;
   assert(block_1024_bytes_segmented_vector_t::get_block_size() == 1024u/sizeof(int));

   //This segmented_vector will allocate blocks of 1024 bytes (an alias for block_bytes)
   typedef segmented_vector<int, void, segment_1024_bytes_option_t > segment_1024_bytes_segmented_vector_t;
   assert(segment_1024_bytes_segmented_vector_t::get_block_size() == 1024u/sizeof(int));

   return 0;
}
//]
