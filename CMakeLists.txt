# -*- shell-script -*- make up a mode for emacs

# To use cmake:
#
# cmake .
# make

cmake_minimum_required(VERSION 3.25)
set(CMAKE_POLICY_VERSION_MINIMUM "3.5")
project (
  libais
  DESCRIPTION "A library for parsing Automatic Identification System messages."
  LANGUAGES CXX
  VERSION 0.17
)

include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++14" COMPILER_SUPPORTS_CXX14)
CHECK_CXX_COMPILER_FLAG("-std=c++17" COMPILER_SUPPORTS_CXX17)
CHECK_CXX_COMPILER_FLAG("-std=c++20" COMPILER_SUPPORTS_CXX20)
if(COMPILER_SUPPORTS_CXX20)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20")
elseif(COMPILER_SUPPORTS_CXX17)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
elseif(COMPILER_SUPPORTS_CXX14)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
else()
        message(STATUS "The compiler ${CMAKE_CXX_COMPILER} is too old. Please use a different C++ compiler.")
endif()

option(ENABLE_ASAN "Enable AddressSanitizer (ASan)" OFF)
if(ENABLE_ASAN)
    add_compile_options(-fsanitize=address -g -fno-omit-frame-pointer)
    add_link_options(-fsanitize=address)
endif()

include(CTest)
if(BUILD_TESTING)
    add_compile_options(-Wno-deprecated-copy)
    add_subdirectory(third_party/gmock)
endif()

# include_directories("${PROJECT_BINARY_DIR}/src/libais")

add_subdirectory(src)

