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

include_directories("${PROJECT_BINARY_DIR}")

add_library(ais SHARED STATIC ais ais1_2_3 ais4_11 ais5 ais7_13 ais8 ais9 ais10 ais12 ais14 ais15 ais18 ais19 ais21 ais24 ais8_366_22 )

add_executable(test_libais test_libais)
target_link_libraries (test_libais ais)

# There must be a new line after each command!
