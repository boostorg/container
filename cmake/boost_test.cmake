# Copyright 2018 Peter Dimov
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt

function(boost_test)

  cmake_parse_arguments(
    _
    ""
    "TYPE;PREFIX;NAME"
    "SOURCES;LIBRARIES;ARGUMENTS"
    ${ARGN}
  )

  if(NOT __TYPE)
    set(__TYPE run)
  endif()

  if(NOT __PREFIX)
    set(__PREFIX ${PROJECT_NAME})
  endif()

  if(NOT __NAME)
    list(GET __SOURCES 0 __NAME)
    string(MAKE_C_IDENTIFIER ${__NAME} __NAME)
  endif()

  set(__NAME ${__PREFIX}-${__NAME})

  if(__TYPE STREQUAL "compile" OR __TYPE STREQUAL "compile-fail")

    add_library(${__NAME} EXCLUDE_FROM_ALL ${__SOURCES})
    target_link_libraries(${__NAME} ${__LIBRARIES})

    add_test(NAME compile-${__NAME} COMMAND "${CMAKE_COMMAND}" --build ${CMAKE_BINARY_DIR} --target ${__NAME})

    if(__TYPE STREQUAL "compile-fail")
      set_tests_properties(compile-${__NAME} PROPERTIES WILL_FAIL TRUE)
    endif()

  elseif(__TYPE STREQUAL "link")

    add_executable(${__NAME} EXCLUDE_FROM_ALL ${__SOURCES})
    target_link_libraries(${__NAME} ${__LIBRARIES})

    add_test(NAME link-${__NAME} COMMAND "${CMAKE_COMMAND}" --build ${CMAKE_BINARY_DIR} --target ${__NAME})

  elseif(__TYPE STREQUAL "link-fail")

    add_library(compile-${__NAME} EXCLUDE_FROM_ALL ${__SOURCES})
    target_link_libraries(compile-${__NAME} ${__LIBRARIES})

    add_test(NAME compile-${__NAME} COMMAND "${CMAKE_COMMAND}" --build ${CMAKE_BINARY_DIR} --target compile-${__NAME})

    add_executable(${__NAME} EXCLUDE_FROM_ALL ${__SOURCES})
    target_link_libraries(${__NAME} ${__LIBRARIES})

    add_test(NAME link-${__NAME} COMMAND "${CMAKE_COMMAND}" --build ${CMAKE_BINARY_DIR} --target ${__NAME})
    set_tests_properties(link-${__NAME} PROPERTIES WILL_FAIL TRUE)

  elseif(__TYPE STREQUAL "run" OR __TYPE STREQUAL "run-fail")

    add_executable(${__NAME} ${__SOURCES})
    target_link_libraries(${__NAME} ${__LIBRARIES})

    add_test(NAME compile-${__NAME} COMMAND "${CMAKE_COMMAND}" --build ${CMAKE_BINARY_DIR} --target ${__NAME})

    message(TRACE "add_test(${__NAME})")
    add_test(NAME run-${__NAME} COMMAND ${__NAME} ${__ARGUMENTS})
    set_tests_properties(run-${__NAME} PROPERTIES DEPENDS compile-${__NAME})

    if(__TYPE STREQUAL "run-fail")
      set_tests_properties(run-${__NAME} PROPERTIES WILL_FAIL TRUE)
    endif()

  endif()

endfunction()

function(boost_test_jamfile)

  cmake_parse_arguments(
    _
    ""
    "FILE;PREFIX"
    "LIBRARIES"
    ${ARGN}
  )

  file(STRINGS ${__FILE} data)

  set(types
      compile
      compile-fail
      link
      link-fail
      run
      run-fail
  )

  foreach(line IN LISTS data)
    if(line)

      string(REGEX MATCHALL "[^ ]+" ll ${line})

      if(ll)
        list(GET ll 0 e0)

        if(e0 IN_LIST types)

          list(LENGTH ll lln)

          if(NOT lln EQUAL 2)

            message(WARNING "Jamfile line ignored: ${line}")

          else()

            list(GET ll 1 e1)
            message(TRACE "boost_test(PREFIX ${__PREFIX} TYPE ${e0} SOURCES ${e1} LIBRARIES ${__LIBRARIES})")
            boost_test(PREFIX ${__PREFIX} TYPE ${e0} SOURCES ${e1} LIBRARIES ${__LIBRARIES})

          endif()
        endif()
      else()
        message(WARNING "${line}")
      endif()
    endif()
  endforeach()

endfunction()
