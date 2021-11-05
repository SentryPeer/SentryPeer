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
## Updated: 2021-10-26
## Author: Gavin Henry <ghenry@sentrypeer.org (GH)
##
######

bin_PROGRAMS=sentrypeer
sentrypeer_SOURCES= \
    src/sentrypeer.c \
    src/sentrypeer.h \
    src/conf.c \
    src/conf.h \
    src/sip_daemon.c \
    src/sip_daemon.h \
    src/sip_parser.c \
    src/sip_parser.h

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
# -g and -O2 already added by default
# AM_CFLAGS=-Wall -Wextra -Werror -std=c18 -pedantic
# https://github.com/mcinglis/c-style
AM_CFLAGS = -Wall -Werror -Wextra -Wpedantic -pedantic \
          -Wformat=2 -Wno-unused-parameter -Wshadow \
          -Wwrite-strings -Wstrict-prototypes -Wold-style-definition \
          -Wredundant-decls -Wnested-externs -Wmissing-include-dirs \
          -std=c18


AM_LDFLAGS=
ACLOCAL_AMFLAGS=-I m4

# Testing - https://www.gnu.org/software/automake/manual/html_node/Scripts_002dbased-Testsuites.html#Scripts_002dbased-Testsuites
# TESTS=test1.sh
