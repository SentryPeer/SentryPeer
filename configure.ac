# SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only
# Copyright (c) 2021 Gavin Henry <ghenry@sentrypeer.org>

#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([sentrypeer], [0.0.1], [ghenry@sentrypeer.org],,[https://github.com/SentryPeer/SentryPeer])
AC_CONFIG_HEADERS([config.h])
AC_LANG(C)
AC_CONFIG_SRCDIR([src/sentrypeer.h])
AC_CONFIG_MACRO_DIRS([m4])

# Checks for programs.
AM_INIT_AUTOMAKE([-Wall -Werror foreign subdir-objects])
AM_SILENT_RULES([yes])
AC_PROG_CC
AC_PROG_LIBTOOL

# Check for libraries.
#AC_SEARCH_LIBS(SHA1, crypto, [], [
#  AC_MSG_ERROR([unable to find the SHA1() function. openssl libs installed?])
#])

#AC_SEARCH_LIBS(curl_easy_init, curl, [], [
#  AC_MSG_ERROR([unable to find the curl_easy_init() function. curl libs installed?])
#])

#AC_SEARCH_LIBS(json_object, jansson, [], [
#  AC_MSG_ERROR([unable to find the json_object() function. jansson libs installed?])
#])

# Checks for header files.
AC_CHECK_HEADERS([unistd.h stdio.h stdlib.h stdbool.h errno.h string.h])

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.
AC_FUNC_MALLOC

AC_SUBST(VERSION)

AC_CHECK_PROG(GIT, git, 1.7.0, [
 AC_MSG_ERROR([unable to find the git program. git installed?])
])

if [[ ! -z "${GIT}" ]]; then
	REVISION=`git rev-parse --short HEAD`
else
	REVISION="not found."
fi

AC_DEFINE_UNQUOTED(REVISION, "$REVISION", [Git revision])
AC_SUBST(REVISION)

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
