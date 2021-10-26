# SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only
# Copyright (c) 2021 Gavin Henry <ghenry@sentrypeer.org>

#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([sentrypeer], [0.0.1], [ghenry@sentrypeer.org],[https://www.sentrpeer.org],[https://github.com/SentryPeer/SentryPeer])
AC_LANG(C)
AC_CONFIG_SRCDIR([src/sentrypeer.h])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIRS([m4])

# Checks for programs.
AM_INIT_AUTOMAKE([foreign subdir-objects])
AM_SILENT_RULES([yes])
AC_PROG_CC
AC_PROG_LIBTOOL

# Check for libraries.
AC_SEARCH_LIBS(SHA1, crypto, [], [
  AC_MSG_ERROR([unable to find the SHA1() function. openssl libs installed?])
])

AC_SEARCH_LIBS(curl_easy_init, curl, [], [
  AC_MSG_ERROR([unable to find the curl_easy_init() function. curl libs installed?])
])

AC_SEARCH_LIBS(json_object, jansson, [], [
  AC_MSG_ERROR([unable to find the json_object() function. jansson libs installed?])
])

# Checks for header files.
AC_CHECK_HEADERS([stdlib.h string.h curl/curl.h jansson.h openssl/sha.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL

# Checks for library functions.
AC_FUNC_MALLOC

# Our versions follow https://semver.org
SENTRYPEER_VERSION=001
AC_SUBST(SENTRYPEER_VERSION)
SENTRYPEER_VERSION_STRING="0.0.1"
AC_SUBST(SENTRYPEER_VERSION_STRING)

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
