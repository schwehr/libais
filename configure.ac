# libais autoconf build setup.
#
# autoreconf -v --force --install -I m4 && ./configure
# libtoolize --force --copy

AC_PREREQ([2.65])
AC_INIT([libais],[0.15],[schwehr@gmail.com])


AC_CONFIG_SRCDIR([src/libais/ais.cpp])
AC_CONFIG_MACRO_DIRS([m4])


AM_INIT_AUTOMAKE
AM_MAINTAINER_MODE

AC_PROG_CXX
AC_PROG_CXXCPP
AC_LANG([C++])
AX_CXX_COMPILE_STDCXX_11([noext], [mandatory])

AC_PROG_INSTALL

AC_HEADER_STDC
AC_CHECK_HEADERS([array])
AC_CHECK_HEADERS([bitset])
AC_CHECK_HEADERS([cassert])
AC_CHECK_HEADERS([cmath])
AC_CHECK_HEADERS([fstream])
AC_CHECK_HEADERS([iostream])
AC_CHECK_HEADERS([memory])
AC_CHECK_HEADERS([sstream])
AC_CHECK_HEADERS([vector])
AC_CHECK_HEADERS([string])
AC_CHECK_HEADERS([vector])

# Check for signed char.
AC_TYPE_INT8_T

LT_INIT
AC_SUBST([LIBTOOL_DEPS])
LT_LIB_M

AC_CONFIG_FILES([Makefile])
AC_CONFIG_FILES([src/Makefile])
AC_CONFIG_FILES([src/libais/Makefile])
AC_OUTPUT
