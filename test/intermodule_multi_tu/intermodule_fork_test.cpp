//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2026-2026. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////
//intermodule_globals<> and the dlmalloc heap in the child of fork(): this
//executable and two shared libraries share one object for each T/Options pair,
//and the child must still share it, with the parent's values. The heap must
//still work in the child, including blocks that the parent allocated.
//
//On Cygwin the registry is published through a named Win32 section whose name
//contains the Windows process id. The child has a new id, so the section must
//be published again in the child (see intermodule_registry_atfork_child).
#include "intermodule_test_types.hpp"
#include <boost/container/detail/dlmalloc.hpp>
#include "../lightweight_test.hpp"

//fork() is mandatory in POSIX, but some systems with <unistd.h> do not support
//it (e.g. VxWorks, INTEGRITY, Emscripten, WASI). Test only where it works.
#if defined(__linux__) || defined(__CYGWIN__) || defined(__FreeBSD__) || defined(__NetBSD__) \
 || defined(__OpenBSD__) || defined(__DragonFly__) || defined(__sun) || defined(_AIX)        \
 || defined(__QNX__) || (defined(__APPLE__) && defined(__MACH__))
#  define BOOST_CONTAINER_TEST_HAS_FORK
#endif

#if defined(__APPLE__) && defined(BOOST_CONTAINER_TEST_HAS_FORK)
#  include <TargetConditionals.h>
#  if !TARGET_OS_OSX
#     undef BOOST_CONTAINER_TEST_HAS_FORK   //iOS, tvOS and watchOS forbid fork()
#  endif
#endif

#if defined(BOOST_CONTAINER_TEST_HAS_FORK)

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <cerrno>
#include <cstdlib>

namespace dtl = boost::container::dtl;

//Exported by intermodule_lib_a.cpp / intermodule_lib_b.cpp
BOOST_CONTAINER_TEST_INTERMODULE_LIB_DECL(lib_a)
BOOST_CONTAINER_TEST_INTERMODULE_LIB_DECL(lib_b)

namespace {

bc_test::shared_globals &exe_alpha()
{  return dtl::intermodule_globals<bc_test::shared_globals, bc_test::options_alpha>();  }

const int parent_value = 4242;
const int child_value  = 777;
const std::size_t block_size = 100u;

//The three modules share the alpha object, with the expected value
void check_shared_alpha(int expected_value)
{
   BOOST_TEST(&exe_alpha() == lib_a_alpha());
   BOOST_TEST(&exe_alpha() == lib_b_alpha());
   BOOST_TEST(exe_alpha().magic == bc_test::shared_globals::magic_value);
   BOOST_TEST(exe_alpha().self  == &exe_alpha());
   BOOST_TEST_EQ(lib_a_alpha_value(), expected_value);
   BOOST_TEST_EQ(lib_b_alpha_value(), expected_value);
}

#if defined(BOOST_CONTAINER_INTERMODULE_REGISTRY_IN_MMAP)

//The registry is published under this process's section name, and the shared
//objects live in the published registry
void check_registry_published()
{
   void *section = 0;
   dtl::intermodule_registry_header *const view = dtl::intermodule_map_registry_section(section);
   BOOST_TEST(view->magic == dtl::intermodule_registry_magic);
   const char *const registry = static_cast<const char *>(view->canonical);
   const char *const obj      = reinterpret_cast<const char *>(&exe_alpha());
   BOOST_TEST(registry != 0);
   BOOST_TEST(obj > registry && obj < registry + BOOST_CONTAINER_INTERMODULE_REGISTRY_BYTES);
   (void)UnmapViewOfFile(view);
   (void)CloseHandle(section);
}

#else

void check_registry_published()
{}

#endif

//Runs in a child: everything the parent shared is still shared, the heap
//works, and blocks of the parent can be freed
void child_checks(void *parent_block, int depth)
{
   check_registry_published();
   check_shared_alpha(parent_value);

   boost::container::dlmalloc &heap = boost::container::dlmalloc_heap();
   BOOST_TEST(boost::container::dlmalloc::usable_size(parent_block) >= block_size);
   heap.deallocate(parent_block);
   void *const p = heap.allocate(block_size);
   BOOST_TEST(p != 0);
   if(p)
      heap.deallocate(p);

   //Writes are shared in the child too
   lib_a_set_alpha_value(child_value);
   BOOST_TEST_EQ(exe_alpha().value, child_value);
   BOOST_TEST_EQ(lib_b_alpha_value(), child_value);
   lib_a_set_alpha_value(parent_value);

   //A second generation: the child forks again
   if(depth == 1){
      void *const block = heap.allocate(block_size);
      BOOST_TEST(block != 0);
      const pid_t pid = ::fork();
      if(pid == 0){
         child_checks(block, depth + 1);
         std::exit(boost::report_errors());
      }
      BOOST_TEST(pid > 0);
      if(pid > 0){
         int status = 0;
         BOOST_TEST(::waitpid(pid, &status, 0) == pid);
         BOOST_TEST(WIFEXITED(status) && WEXITSTATUS(status) == 0);
      }
      heap.deallocate(block);
   }
}

}  //namespace

int main()
{
   //The parent uses the shared object and the heap before fork()
   lib_a_set_alpha_value(parent_value);
   check_shared_alpha(parent_value);
   check_registry_published();
   boost::container::dlmalloc &heap = boost::container::dlmalloc_heap();
   void *const block = heap.allocate(block_size);
   BOOST_TEST(block != 0);

   const pid_t pid = ::fork();
   if(pid == 0){
      child_checks(block, 1);
      std::exit(boost::report_errors());
   }
   if(pid < 0 && errno == ENOSYS){
      //fork() is not supported at run time
      heap.deallocate(block);
      return boost::report_errors();
   }
   BOOST_TEST(pid > 0);
   if(pid > 0){
      int status = 0;
      BOOST_TEST(::waitpid(pid, &status, 0) == pid);
      BOOST_TEST(WIFEXITED(status) && WEXITSTATUS(status) == 0);
   }

   //The parent is not affected by the child
   check_shared_alpha(parent_value);
   check_registry_published();
   heap.deallocate(block);
   return boost::report_errors();
}

#else    //BOOST_CONTAINER_TEST_HAS_FORK

int main()
{
   return boost::report_errors();
}

#endif   //BOOST_CONTAINER_TEST_HAS_FORK
