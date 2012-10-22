# -*- shell-script -*- make up a mode for emacs
# CMake setup primarily by Roland Arsenault - https://github.com/rolker/libais

# To use cmake:

# cmake .
# make

cmake_minimum_required (VERSION 2.8)
project (libais)

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
    ais8 ais8_001_22 ais8_001_26 ais8_366_22
    ais9
    ais10 # :
    # ais11 ; - See 4
    ais12 # <
    # ais13 = See 7
    ais14 # >
    ais15 # ?
    ais16 # @
    ais17 # A
    ais18 # B
    ais19 # C
    ais20 # D
    ais21 # E
    ais22 # F
    ais23 # G
    ais24 # H
    ais25 # I
    ais26 # J
    ais27 # K
    # ais28 # L - Not yet defined
)

install(FILES ${HEADERS} DESTINATION include)

add_library(ais STATIC  ${SOURCES} )
install(TARGETS ais DESTINATION lib)

option(AIS_PYTHON "Build python bindings" ON)

if(AIS_PYTHON)
    find_package(PythonInterp REQUIRED)
    set (PYTHON_SOURCES ${SOURCES} ais_py.cpp)
    add_custom_target(python_build ALL ${PYTHON_EXECUTABLE} setup.py build --build-base ${CMAKE_BINARY_DIR}/python
                       DEPENDS ${SOURCES}
                       WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                       COMMENT "python build"
                       SOURCES ${PYTHON_SOURCES}
                      )

    install(CODE "message(\"python install\")\n execute_process(COMMAND ${PYTHON_EXECUTABLE} setup.py -q build --build-base ${CMAKE_BINARY_DIR}/python install --prefix ${CMAKE_INSTALL_PREFIX} WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})")
endif()

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
        ais8_200_10_unittest
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

option(BUILD_OLD_TESTS  "Enable old testing code." OFF)

if(BUILD_OLD_TESTS)
    add_executable(test_libais test_libais.cpp)
    target_link_libraries(test_libais ais)
    add_test(test_libais test_libais)
endif()