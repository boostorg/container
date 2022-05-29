include(../cmake/boost_test.cmake)

file(GLOB_RECURSE sources "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

foreach(file IN LISTS sources)
  boost_test(TYPE run SOURCES ${file} LIBRARIES boost_container)
endforeach()
