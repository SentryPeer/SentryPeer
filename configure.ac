# SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only  */
# Copyright (c) 2021 - 2025 Gavin Henry <ghenry@sentrypeer.org> */
#
#   _____            _              _____
#  / ____|          | |            |  __ \
# | (___   ___ _ __ | |_ _ __ _   _| |__) |__  ___ _ __
#  \___ \ / _ \ '_ \| __| '__| | | |  ___/ _ \/ _ \ '__|
#  ____) |  __/ | | | |_| |  | |_| | |  |  __/  __/ |
# |_____/ \___|_| |_|\__|_|   \__, |_|   \___|\___|_|
#                              __/ |
#                             |___/
#
#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([sentrypeer],[4.0.5],[ghenry@sentrypeer.org],[],[https://github.com/SentryPeer/SentryPeer])
# Some weird issue with AX_PTHREAD macro that needs this to be called first:
AC_CANONICAL_TARGET
AM_INIT_AUTOMAKE([-Wall -Werror foreign subdir-objects])
AC_USE_SYSTEM_EXTENSIONS
AC_CONFIG_SRCDIR([src/sentrypeer.h])
AC_CONFIG_HEADERS([config.h])
AC_LANG(C)
AC_CONFIG_MACRO_DIRS([m4])

# Checks for programs.
#AM_SILENT_RULES([yes])
# For Rust to call sentrypeer functions etc.
AM_PROG_AR
LT_INIT

AC_PROG_CXX
AC_PROG_AWK
AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET

# Check for libraries.
PKG_CHECK_MODULES(libosip2, libosip2 >= 5.0, [], [
  AC_MSG_WARN([It is highly recommended to install libosip2 >= 5.0])
])

AC_SEARCH_LIBS(osip_message_init, osipparser2, [], [
  AC_MSG_ERROR([osip_message_init() is not available. libosip2-dev / libosip2-devel or equivalent is required.])
])

AC_SEARCH_LIBS(sqlite3_open, sqlite3, [], [
  AC_MSG_ERROR([sqlite3_open() is not available. libsqlite3-dev / sqlite-devel or equivalent is required.])
])

AC_SEARCH_LIBS(uuid_generate, uuid, [], [
  AC_MSG_ERROR([uuid_generate() is not available. uuid-dev / libuuid-devel or equivalent is required.])
])

AC_SEARCH_LIBS(json_object, jansson, [], [
  AC_MSG_ERROR([json_object() is not available. libjansson-dev / jansson-devel or equivalent is required.])
])

AC_SEARCH_LIBS(curl_easy_init, curl, [], [
  AC_MSG_ERROR([curl_easy_init() is not available. libcurl-dev / libcurl-devel or equivalent is required.])
])

AC_SEARCH_LIBS(MHD_start_daemon, microhttpd, [], [
  AC_MSG_ERROR([MHD_start_daemon() is not available. libmicrohttpd-dev / libmicrohttpd-devel or equivalent is required.])
])

AC_SEARCH_LIBS(pcre2_compile_8, pcre2-8, [], [
  AC_MSG_ERROR([pcre2_compile_8() is not available. libpcre2-dev / pcre2-devel or equivalent is required.])
])

AC_SEARCH_LIBS(pthread_create, pthread, [], [
  AC_MSG_ERROR([pthread_create() is not available.])
])

AC_SEARCH_LIBS(ceil, m, [], [
  AC_MSG_ERROR([ceil() is not available.])
])

# Checks for header files.
AC_CHECK_HEADERS([arpa/inet.h netdb.h netinet/in.h stddef.h stdint.h stdlib.h string.h strings.h sys/socket.h sys/time.h syslog.h unistd.h signal.h pthread.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_TYPE_INT64_T
AC_TYPE_SIZE_T
AC_TYPE_UINT32_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_FUNC_STRNLEN
AC_CHECK_FUNCS([setenv clock_gettime getcwd inet_ntoa localtime_r memset select socket strdup strncasecmp strndup atexit bzero])

# https://github.com/stedolan/jq/blob/master/configure.ac#L129
# Find pthread, if we have it. We do this first because we may set -pthread on CFLAGS
# which can cause various macros to be defined (__REENTRANT on Darwin, for example)
AX_PTHREAD([
   AC_DEFINE([HAVE_PTHREAD], [1], [Define if pthread is available])
   LIBS="$PTHREAD_LIBS $LIBS"
   CFLAGS="$CFLAGS $PTHREAD_CFLAGS"
   CC="$PTHREAD_CC"
],
[AC_MSG_ERROR([POSIX threads support is required])])

PKG_CHECK_MODULES([OPENDHT_C], [opendht-c], [
   AC_MSG_CHECKING([we have opendht-c >= 2.3.5])
   PKG_CHECK_EXISTS(opendht-c >= 2.3.5, [
      opendht_c_version=`pkg-config --modversion opendht-c`
      AC_DEFINE_UNQUOTED([OPENDHT_C_VERSION], ["$opendht_c_version"], [Define the OpenDHT version])
      AC_MSG_RESULT([yes, version $opendht_c_version])
      AC_DEFINE([HAVE_OPENDHT_C], [1], [Define if OpenDHT is available])
      LIBS="$OPENDHT_C_LIBS $LIBS"
      CFLAGS="$CFLAGS $OPENDHT_C_CFLAGS"
   ],
   [
      AC_MSG_RESULT([no])
      AC_MSG_ERROR([We need at least version 2.3.5 of OpenDHT-C.
Use --disable-opendht if you don't want Peer to Peer support.])
   ])
],
[AC_MSG_WARN([OpenDHT-c is not detected via pkg-config. Please install it manually.])])
AM_CONDITIONAL([HAVE_OPENDHT_C], [test "$HAVE_OPENDHT_C" = "1"])

AC_CHECK_PROG(GIT, git, 1.7.0, [
 AC_MSG_ERROR([unable to find the git program. git installed?])
])

AC_MSG_CHECKING(for IP_PKTINFO)
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <netdb.h>]], [[
  int opt = IP_PKTINFO;
]])],[
  AC_MSG_RESULT(yes)
  AC_DEFINE(HAVE_IP_PKTINFO, 1, [Have IP_PKTINFO])
],[
  AC_MSG_RESULT(no)
])

AC_MSG_CHECKING(for IP_RECVDSTADDR)
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <netinet/in.h>]], [[
  int opt = IP_RECVDSTADDR;
]])],[
  AC_MSG_RESULT(yes)
  AC_DEFINE(HAVE_IP_RECVDSTADDR, 1, [Have IP_RECVDSTADDR])
],[
  AC_MSG_RESULT(no)
])

# Allow a user to disable OpenDHT, i.e. the Peer to Peer elements of SentryPeer for a
# full standalone node (we've reversed logic so the default is to have these enabled)
AC_MSG_CHECKING(if you'd like to disable OpenDHT support)
AC_ARG_ENABLE([opendht],
    AS_HELP_STRING([--disable-opendht], [Do not build with OpenDHT support]),
    [disable_opendht="yes"],
    [disable_opendht="no"])
AC_MSG_RESULT($disable_opendht)
if test "$disable_opendht" = "yes"; then
  AC_DEFINE([HAVE_OPENDHT_C], [0], [Disable OpenDHT support])
  AC_MSG_WARN([OpenDHT support is disabled. Distributed Hash Tables will not be available])
fi
AM_CONDITIONAL([DISABLE_OPENDHT], [test x$disable_opendht = xyes])

if [[ ! -z "${GIT}" ]]; then
	REVISION=`git rev-parse --short HEAD`
else
	REVISION="not found."
fi

AC_DEFINE_UNQUOTED(REVISION, "$REVISION", [Git revision])
AC_SUBST(REVISION)

### For our Rust parts
# https://www.figuiere.net/hub/blog/?2016/10/07/862-rust-and-automake
# https://ljones.dev/blog/gsoc-2/
# https://viruta.org/librsvgs-build-infrastructure-autotools-and-rust.html
AC_PATH_PROG([CARGO], [cargo], [notfound])
AS_IF(test "$CARGO" = "notfound",
    AC_DEFINE([HAVE_RUST], [0], [Rust not found])
    AC_MSG_WARN([cargo is required for Rust parts. Please install the Rust toolchain from https://www.rust-lang.org/])
)
AS_IF(test "$CARGO" != "notfound",
    AC_MSG_CHECKING([for Cargo version])
    CARGO_VERSION=`$CARGO --version | awk '{print $2}'`
    AC_MSG_RESULT($CARGO_VERSION)
    AC_DEFINE_UNQUOTED([CARGO_VERSION], ["$CARGO_VERSION"], [Define the Cargo version])
    AC_DEFINE([HAVE_RUST], [1], [Cargo found])
)
AM_CONDITIONAL([HAVE_RUST], [test x$CARGO != xnotfound])

AC_PATH_PROG([RUSTC], [rustc], [notfound])
AS_IF(test "$RUSTC" = "notfound",
    AC_DEFINE([HAVE_RUST], [0], [Rust not found])
    AC_MSG_WARN([rustc is required for Rust parts. Please install the Rust toolchain from https://www.rust-lang.org/])
)
AS_IF(test "$RUSTC" != "notfound",
    AC_MSG_CHECKING([for Rust compiler version])
    RUSTC_VERSION=`$RUSTC --version | awk '{print $2}'`
    AC_MSG_RESULT($RUSTC_VERSION)
    AC_DEFINE_UNQUOTED([RUSTC_VERSION], ["$RUSTC_VERSION"], [Define the Rust compiler version])
    AC_DEFINE([HAVE_RUST], [1], [Rust found])
)
AM_CONDITIONAL([HAVE_RUST], [test x$RUSTC != xnotfound])

# Allow a user to disable Rust parts
AC_MSG_CHECKING(if you'd like to disable Rust parts)
AC_ARG_ENABLE([rust],
    AS_HELP_STRING([--disable-rust], [Do not build Rust parts]),
    [disable_rust="yes"],
    [disable_rust="no"])
AC_MSG_RESULT($disable_rust)
AS_IF(test "$disable_rust" = "yes",
  AC_DEFINE([HAVE_RUST], [0], [Disable Rust parts])
  AC_MSG_WARN([Rust parts are disabled. Certain features will not be available])
)
AM_CONDITIONAL([DISABLE_RUST], [test x$disable_rust = xyes])

# https://viruta.org/librsvgs-build-infrastructure-autotools-and-rust.html
AC_MSG_CHECKING(whether to build Rust parts with debugging information)
AC_ARG_ENABLE([rust_debug],
    AS_HELP_STRING([--enable-rust-debug],
    [Build Rust code with debugging information [default=no]]),
    [rust_debug_release=$enableval],
    [rust_debug_release=no])

if test "x$rust_debug_release" = "xyes" ; then
    AC_MSG_RESULT(yes)
    RUST_TARGET_SUBDIR=debug
else
    AC_MSG_RESULT(no)
    RUST_TARGET_SUBDIR=release
fi
AM_CONDITIONAL([RUST_DEBUG_RELEASE], [test "x$rust_debug_release" = "xyes"])

AC_SUBST([RUST_TARGET_SUBDIR])
### End Rust parts

# Ideas taken from https://github.com/staskobzar/libamip/blob/master/configure.ac
#
# Doxygen
AC_CHECK_PROGS([DOXYGEN], [doxygen])
if test -z "$DOXYGEN";
   then AC_MSG_WARN([Doxygen not found - continuing without Doxygen support])
fi
AM_CONDITIONAL([HAVE_DOXYGEN], [test -n "$DOXYGEN"])

# CMOCKA unit tests
PKG_CHECK_EXISTS(cmocka >= 1.1.5,
		[AC_CHECK_HEADERS([stdarg.h stddef.h setjmp.h],
			[], dnl We are only interested in action-if-not-found
			[AC_MSG_WARN([Header files stdarg.h stddef.h setjmp.h are required by cmocka])
			 cmocka_required_headers="no"
			]
		)
		AS_IF([test x"$cmocka_required_headers" != x"no"],
			[PKG_CHECK_MODULES([CMOCKA], [cmocka], [have_cmocka="yes"])]
		)],
		dnl PKG_CHECK_EXISTS ACTION-IF-NOT-FOUND
		[AC_MSG_WARN([No libcmocka-1.1.5 or newer library found, cmocka tests will not be built])]
	)
AM_CONDITIONAL([HAVE_CMOCKA], [test x$have_cmocka = xyes])

# Test coverage generate. Flag: --with-coverage
AC_ARG_WITH([coverage],
			AS_HELP_STRING([--with-coverage], [Generate test coverage report with lcov.]),
			[with_cov=true], []
			)

AM_CONDITIONAL([WITH_COVERAGE], [test x$with_cov = xtrue])
if test x$with_cov = xtrue; then
  AC_PATH_PROG(LCOV, lcov)
  AC_PATH_PROG(GCOV, gcov)
  AC_PATH_PROG(GENHTML, genhtml)
  COVERAGE_CFLAGS="--coverage"
  COVERAGE_OPTFLAGS="-O0"
  AC_SUBST([GCOV])
  AC_SUBST([LCOV])
  AC_SUBST([GENHTML])
  AC_SUBST([COVERAGE_CFLAGS])
  AC_SUBST([COVERAGE_OPTFLAGS])
fi

# Add these flags in for testing as per p219 of Effective C PDF
# Enable AddressSanitizer. Flag: --with-asan
AC_ARG_WITH([asan],
			AS_HELP_STRING([--with-asan], [Compile tests with AddressSanitizer (aka ASan)]),
			[with_asan=true], []
			)

AM_CONDITIONAL([WITH_ASAN], [test x$with_asan = xtrue])
if test x$with_asan = xtrue; then
  LIBASAN_FLAGS="-fno-omit-frame-pointer -fsanitize=address"

  AX_CHECK_COMPILE_FLAG(
    [[-fsanitize=address]],
    [[LIBASAN_CFLAGS=-fsanitize=address]],
    AC_MSG_ERROR([compiler did not accept requested flags]))

  AX_CHECK_LINK_FLAG(
    [[-fsanitize=address]],
    [[LIBASAN_CFLAGS=-fsanitize=address]],
    [AC_MSG_ERROR([linker did not accept requested flags, you are missing libasan])])

  AX_CHECK_COMPILE_FLAG(
    [-fno-omit-frame-pointer],
    [LIBASAN_CFLAGS="$LIBASAN_CFLAGS -fno-omit-frame-pointer"],
    AC_MSG_ERROR([Cannot enable -fno-omit-frame-pointer]))

  AC_SUBST([LIBASAN_FLAGS])
fi

AC_CONFIG_FILES([Makefile])
AC_OUTPUT

AC_CANONICAL_HOST

# https://github.com/zeromq/zyre/blob/master/configure.ac#L691
BUILD_USER=${USER}
BUILD_ARCH=${host}
BUILD_HOST=${ac_hostname}
BUILD_DATE=$(date +'%F %H:%M')

AC_DEFINE_UNQUOTED([BUILD_USER],     "${BUILD_USER}",     [The fine user who built the package])
AC_DEFINE_UNQUOTED([BUILD_ARCH],     "${BUILD_ARCH}",     [Architecture of the build host])
AC_DEFINE_UNQUOTED([BUILD_HOST],     "${BUILD_HOST}",     [Build host name])
AC_DEFINE_UNQUOTED([BUILD_DATE],     "${BUILD_DATE}",     [Build date])

AC_SUBST(BUILD_USER)
AC_SUBST(BUILD_ARCH)
AC_SUBST(BUILD_HOST)
AC_SUBST(BUILD_DATE)

PROJECT_NAME="sentrypeer"
AC_MSG_NOTICE([
Configured build options for $PROJECT_NAME:
  Configure date:     $BUILD_DATE
  Build arch:         $BUILD_ARCH
  Build user:         $BUILD_USER
  Build host:         $BUILD_HOST
  Install prefix:     ${prefix}
  Compiler:         
   C compiler         ${CC}
   CFLAGS:            ${CFLAGS} ${LIBASAN_FLAGS}
   CPPFLAGS:          ${CPPFLAGS}
   LDFLAGS:	      ${LDFLAGS}
   LIBS:              ${LIBS}

  Configure complete! Now proceed with:
  - 'make'            compile $PROJECT_NAME
  - 'make check'      run the $PROJECT_NAME self tests
  - 'make install'    install the '$PROJECT_NAME' project to $prefix
])
