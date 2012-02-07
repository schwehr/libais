# -*- shell-script -*- make up a mode for emacs
cmake_minimum_required (VERSION 2.8)
project (libais)

set (LIBAIS_VERSION_MAJOR 0)
set (LIBAIS_VERSION_MINOR 6)

# Was driving me nuts in rough development.  Need to restructure
#configure_file (
#  "${PROJECT_SOURCE_DIR}/ais.h.in"
#  "${PROJECT_BINARY_DIR}/ais.h"
#  )

#include_directories("${PROJECT_BINARY_DIR}")



set (HEADERS
    ais.h
    ais8_001_22.h
    ais8_366_22.h
)

set (SOURCES
    ais
    ais1_2_3
    ais4_11
    ais5
    ais7_13
    ais8 ais8_001_22 ais8_366_22
    ais9
    ais10
    ais12
    ais14
    ais15
    ais18
    ais19
    ais21
    ais24
)

install(FILES ${HEADERS} DESTINATION include)

add_library(ais STATIC  ${SOURCES} )
install(TARGETS ais DESTINATION lib)


# testing support

option(BUILD_TESTS  "Enable testing." OFF)

if(BUILD_TESTS)

    option(TESTS_NEED_PTHREADS "Enable if tests need to link agains pthreads" OFF)
    option(SEPARATE_TESTS "Enable if separate tests should be built, otherwise a single test executable will be built containing all tests" ON)

    find_path(gtest_INCLUDE_DIR gtest/test.h)
    find_library(gtest_LIBRARY gtest)
    find_library(gtest_main_LIBRARY gtest_main)

    if(gtest_INCLUDE_DIR AND gtest_LIBRARY AND gtest_main_LIBRARY)
        include_directories(${gtest_INCLUDE_DIR})
        set(TEST_LIBS ${TEST_LIBS} ${gtest_LIBRARY} ${gtest_main_LIBRARY})
    else()
        message(FATAL_ERROR "gtest not found")
    endif()

    if(TESTS_NEED_PTHREADS)
        find_package(Threads)
        if(CMAKE_THREAD_LIBS_INIT AND CMAKE_USE_PTHREADS_INIT)
            set(TEST_LIBS ${TEST_LIBS} ${CMAKE_THREAD_LIBS_INIT})
        else()
            message(FATAL_ERROR "pthreads not found")
        endif()
    endif()

    set (TEST_SOURCES
        ais1_2_3_unittest
        ais8_001_22_unittest
    )

    enable_testing()

    if(SEPARATE_TESTS)
        foreach(TEST_SOURCE ${TEST_SOURCES})
            add_executable(${TEST_SOURCE} ${TEST_SOURCE})
            target_link_libraries (${TEST_SOURCE} ais ${TEST_LIBS})
            add_test(${TEST_SOURCE} ${TEST_SOURCE})
        endforeach()
    else()
        add_executable(all_tests ${TEST_SOURCES})
        target_link_libraries(all_tests ais ${TEST_LIBS})
        add_test(all_tests all_tests)
    endif()

endif()

# There must be a new line after each command!
