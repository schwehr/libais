# -*- Autoconf -*-
# Copyright 2014 Google Inc. All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# Detect installed libais.
#
# Use the AX_LIB_AIS autoconf macro to enable libais support in
# other libraries or programs.
#
# AX_LIB_AIS defaults to not looking for libais (--without-ais or
# --with-ais=no).
#
# Specifying --with-ais assumes that libais is installed in /usr.
# --with-ais=DIR will look for DIR/include/ais.h and
# DIR/lib/libais.{so,a,dylib}.  If either the header or library is
# not found, it will abort the configure script.
#
# When configured, the following variables are available in configure:
#
#   AIS_HOME - Root of the libais tree.
#   AIS_INC - Include directive (-I) needed for libais.
#   AIS_LIB - Library path directive (-L) for libais.
#
# These variables are set for generated files file AC_SUBST:
#
#   AIS_CPPFLAGS - Include directive (-I) needed for libais.
#   AIS_LDFLAGS - Library path directive (-L) for libais.
#   AIS_LIBS - The actual library to link against.
#
# For C++ source, AC_DEFINE sets HAVE_AIS in any config headers.  Use like so:
#
#   #ifdef HAVE_AIS
#   #include <ais.h>
#   #endif

AC_DEFUN([AX_LIB_AIS], [

  AC_ARG_WITH(
      [ais],
      AS_HELP_STRING(
          [--with-ais],
          [Include ais support (ARG=no, yes or libais directory).]))

  AS_IF([test x$with_ais == xyes], [have_ais=yes],
        [test x$with_ais == xno], [have_ais=no],
        [test x$with_ais != x], [have_ais=yes],
        [have_ais=no])

  AC_MSG_CHECKING([for libais])
  AS_IF([test $have_ais == yes], [AC_MSG_RESULT(yes)],
        [AC_MSG_RESULT(skip)])

  AS_IF([test $have_ais == yes], [
    AS_IF([test $with_ais == yes], [AIS_HOME=/usr],
          [AIS_HOME=$with_ais])
    AIS_INCDIR=$AIS_HOME/include
    AIS_LIBDIR=$AIS_HOME/lib

    AIS_INC=-I$AIS_HOME/include
    AIS_LIB=-L$AIS_HOME/lib

    OLD_CPPFLAGS=$CPPFLAGS
    OLD_LDFLAGS=$LDFLAGS
    CPPFLAGS="$CPPFLAGS ${AIS_INC}"
    LDFLAGS="$LDFLAGS ${AIS_LIB}"
    AC_LANG_PUSH([C++])
    AC_CHECK_HEADER([ais.h], [ac_cv_ais_h=yes], [ac_cv_ais_h=no])
    AC_CHECK_LIB([ais], [LibAisVersionMajor],
                 [ac_cv_lib_ais=yes], [ac_cv_lib_ais=no])
    AC_LANG_POP
    CPPFLAGS="$OLD_CPPFLAGS"
    LDFLAGS="$OLD_LDFLAGS"

    # AC_MSG_FAILURE ?
    AS_IF([test $ac_cv_ais_h == no],
          [AC_MSG_ERROR([ais.h not found with $AIS_INC])])
    AS_IF([test $ac_cv_lib_ais == no],
          [AC_MSG_ERROR([libais not found with $AIS_LIB])])

    AC_DEFINE(HAVE_AIS, 1,
              [Use libais for NMEA Automatic Identification System.])

    HAVE_AIS=$have_ais
    AC_SUBST(HAVE_AIS, $HAVE_AIS)
    AC_SUBST(AIS_CPPFLAGS, $AIS_INC)
    AC_SUBST(AIS_LDFLAGS, $AIS_LIB)
    AC_SUBST(AIS_LIBS, -lais)
  ])

])
