include(BoostTest OPTIONAL RESULT_VARIABLE HAVE_BOOST_TEST)

if(NOT HAVE_BOOST_TEST)
  return()
endif()

#Boost.Container plus the libraries the tests include directly
set(BOOST_TEST_LINK_LIBRARIES Boost::container Boost::container_hash Boost::core
    Boost::iterator Boost::tuple Boost::type_traits Boost::utility)
set(BOOST_TEST_COMPILE_OPTIONS -Wall -Wextra -Wpedantic)
if(MSVC)
  #Some tests (devector_test in Debug) need more sections than an object file
  #has by default
  list(APPEND BOOST_TEST_COMPILE_OPTIONS /bigobj)
endif()

#Every .cpp file in this directory is a test, as in Jamfile.v2
file(GLOB tests RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.cpp)
list(REMOVE_ITEM tests hub_api_test.cpp)

foreach(test IN LISTS tests)
  boost_test(SOURCES ${test})
endforeach()

#hub_api_test needs Boost.Interprocess. Naming the target here is what makes
#the superproject add it: its dependency scanner reads the "Boost::" names out
#of this file
find_package(Threads REQUIRED)
boost_test(SOURCES hub_api_test.cpp LINK_LIBRARIES Boost::interprocess Threads::Threads)

#Two-translation-unit tests: one instance must behave the same from both units
boost_test(SOURCES dlmalloc_multi_tu/dlmalloc_multi_tu_test.cpp dlmalloc_multi_tu/dlmalloc_multi_tu_test_other.cpp)
boost_test(SOURCES pmr_multi_tu/pmr_multi_tu_test.cpp pmr_multi_tu/pmr_multi_tu_test_other.cpp)

#dlmalloc and the original dlmalloc_2_8_6.c driving one heap between them
boost_test(SOURCES dlmalloc_upstream_compat/dlmalloc_upstream_compat_test.cpp)

#Intermodule tests: an executable plus two shared libraries, each inlining its
#own copy of Boost.Container. The libraries are SHARED whatever
#BUILD_SHARED_LIBS says, as crossing a module boundary is the point of the test.
#They export with BOOST_SYMBOL_EXPORT, so default visibility must be kept
function(boost_container_intermodule_libs dir stem)
  foreach(lib a b)
    add_library(boost_container_${stem}_lib_${lib} SHARED EXCLUDE_FROM_ALL ${dir}/${stem}_lib_${lib}.cpp)
    target_link_libraries(boost_container_${stem}_lib_${lib} PRIVATE ${BOOST_TEST_LINK_LIBRARIES})
  endforeach()
endfunction()

boost_container_intermodule_libs(intermodule_multi_tu intermodule)
boost_container_intermodule_libs(pmr_multi_tu pmr_intermodule)
boost_container_intermodule_libs(dlmalloc_multi_tu dlmalloc_intermodule)

set(intermodule_libs boost_container_intermodule_lib_a boost_container_intermodule_lib_b)
boost_test(SOURCES intermodule_multi_tu/intermodule_test.cpp LINK_LIBRARIES ${intermodule_libs})
boost_test(SOURCES intermodule_multi_tu/intermodule_chain_test.cpp LINK_LIBRARIES ${intermodule_libs})
#fork() in the child process (the test does nothing where fork() is missing)
boost_test(SOURCES intermodule_multi_tu/intermodule_fork_test.cpp LINK_LIBRARIES ${intermodule_libs})
boost_test(SOURCES pmr_multi_tu/pmr_intermodule_test.cpp
           LINK_LIBRARIES boost_container_pmr_intermodule_lib_a boost_container_pmr_intermodule_lib_b)
boost_test(SOURCES dlmalloc_multi_tu/dlmalloc_intermodule_test.cpp
           LINK_LIBRARIES boost_container_dlmalloc_intermodule_lib_a boost_container_dlmalloc_intermodule_lib_b)
