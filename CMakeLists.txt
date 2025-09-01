# -*- shell-script -*- make up a mode for emacs

# To use cmake:
#
# cmake .
# make

cmake_minimum_required(VERSION 3.25)
project (
  libais
  DESCRIPTION "A library for parsing Automatic Identification System messages."
  LANGUAGES CXX
  VERSION 0.17
)

include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++14" COMPILER_SUPPORTS_CXX14)
CHECK_CXX_COMPILER_FLAG("-std=c++14" COMPILER_SUPPORTS_CXX17)
CHECK_CXX_COMPILER_FLAG("-std=c++14" COMPILER_SUPPORTS_CXX20)
if(COMPILER_SUPPORTS_CXX20)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20")
elseif(COMPILER_SUPPORTS_CXX17)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
elseif(COMPILER_SUPPORTS_CXX14)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
else()
        message(STATUS "The compiler ${CMAKE_CXX_COMPILER} is too old. Please use a different C++ compiler.")
endif()

# include_directories("${PROJECT_BINARY_DIR}/src/libais")

add_subdirectory(src)

