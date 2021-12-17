# SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only  */
# Copyright (c) 2021 Gavin Henry <ghenry@sentrypeer.org> */
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
######
##
## Example from Page 81 of:
##
## 21st Century C, Second Edition by Ben Klemens
##
## Copyright © 2015 Ben Klemens. All rights reserved.
##
## Published by O’Reilly Media, Inc.
##
## Updated: 2021-12-17
## Author: Gavin Henry <ghenry@sentrypeer.org (GH)
##
######

bin_PROGRAMS=sentrypeer
sentrypeer_SOURCES= \
    src/sentrypeer.c \
    src/sentrypeer.h \
    src/conf.c \
    src/conf.h \
    src/http_common.c \
    src/http_common.h \
    src/http_daemon.c \
    src/http_daemon.h \
    src/http_routes.h \
    src/http_health_check_route.c \
    src/http_ip_addresses_route.c \
    src/http_api.c \
    src/http_api.h \
    src/http_ui.c \
    src/http_ui.h \
    src/sip_daemon.c \
    src/sip_daemon.h \
    src/sip_parser.c \
    src/sip_parser.h \
    src/utils.c \
    src/utils.h \
    src/bad_actor.c \
    src/bad_actor.h \
    src/database.c \
    src/database.h

unit_test_sources = \
    src/http_common.c \
    src/http_common.h \
    src/http_daemon.c \
    src/http_daemon.h \
    src/http_routes.h \
    src/http_health_check_route.c \
    src/http_ip_addresses_route.c \
    src/sip_parser.c \
    src/sip_parser.h \
    src/bad_actor.c \
    src/bad_actor.h \
    src/conf.c \
    src/conf.h \
    src/utils.c \
    src/utils.h \
    src/database.c \
    src/database.h \
    tests/unit_tests/test_conf.c \
    tests/unit_tests/test_conf.h \
    tests/unit_tests/test_utils.c \
    tests/unit_tests/test_utils.h \
    tests/unit_tests/test_bad_actor.c \
    tests/unit_tests/test_bad_actor.h \
    tests/unit_tests/test_database.c \
    tests/unit_tests/test_database.h \
    tests/unit_tests/test_http_api.c \
    tests/unit_tests/test_http_api.h \
    tests/unit_tests/runner.c

EXTRA_DIST=AUTHORS README.md CHANGELOG.md \
           COPYRIGHT COPYING LICENSE.GPL-2.0-only \
           LICENSE.GPL-3.0-only CMakeLists.txt \
           cmake/git_revision.cmake cmake/config.h.in

#########
# Clean

MAINTAINERCLEANFILES=configure aclocal.m4 compile \
                     install-sh missing Makefile.in depcomp \
                     stamp-h1 Makefile.in config.guess \
                     config.sub include/Makefile.in \
                     ltmain.sh depcomp libtool config.h.in \
                     config.h.in~ test-driver

maintainer-clean-local: 
	-rm -rf $(top_srcdir)/build
	-rm -rf $(top_srcdir)/m4
	-rm -rf $(top_srcdir)/autom4te.cache

# From http://mij.oltrelinux.com/devel/autoconf-automake/
# what flags you want to pass to the C compiler & linker
#
# https://github.com/mcinglis/c-style
#
# Also see Effective C, page 205 in the PDF (adding -fpie -Wl,-pie fails on Fedora 34 (Workstation Edition) )
# and some from https://developers.redhat.com/blog/2018/03/21/compiler-and-linker-flags-gcc
AM_CFLAGS =-Wall -Werror -Wextra -Wpedantic -pedantic \
          -Wformat=2 -Wno-unused-parameter -Wshadow \
          -Wwrite-strings -Wstrict-prototypes -Wold-style-definition \
          -Wredundant-decls -Wnested-externs -Wmissing-include-dirs \
          -std=c18 -D_FORTIFY_SOURCE=2 -fpie -fpic -g3 -O2 \
          -fstack-protector-strong -grecord-gcc-switches \
          -Werror=format-security -Werror=implicit-function-declaration \
          -Wmisleading-indentation $(LIBASAN_FLAGS)

AM_LDFLAGS=
ACLOCAL_AMFLAGS=-I m4

if HAVE_CMOCKA
TESTS=tests/unit_tests/runner
check_PROGRAMS = tests/unit_tests/runner
tests_unit_tests_runner_SOURCES = $(unit_test_sources)
tests_unit_tests_runner_CFLAGS = $(AM_CFLAGS)
tests_unit_tests_runner_LDADD = -lcmocka
endif

if HAVE_DOXYGEN
# Doxygen
docs:
	doxygen
endif

dist_man_MANS=sentrypeer.1
