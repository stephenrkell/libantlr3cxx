AC_PREREQ([2.69])
AC_INIT([libantlr3cxx], [0.1], [srk31@cl.cam.ac.uk])
AC_CONFIG_SRCDIR([include/antlr3cxx/parser.hpp])
AC_LANG([C++])
AM_INIT_AUTOMAKE([foreign subdir-objects -Wno-portability])
AM_MAINTAINER_MODE
AC_PROG_CXX
AX_CXX_COMPILE_STDCXX_11
AC_CHECK_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_SIZE_T
AC_CHECK_TYPES([ptrdiff_t])
AC_PROG_LN_S
AC_PROG_INSTALL

AC_CHECK_HEADERS([cassert sstream string vector], [], [AC_MSG_FAILURE([required standard headers: cassert sstream string vector])])

AC_OUTPUT([Makefile libantlr3cxx.pc])
