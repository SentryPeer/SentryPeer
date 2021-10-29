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
    src/conf.h

# From http://mij.oltrelinux.com/devel/autoconf-automake/
# what flags you want to pass to the C compiler & linker
#
# -g and -O2 already added by default
AM_CFLAGS=-Wall -Wextra -Werror -std=c18 -pedantic
AM_LDFLAGS=
ACLOCAL_AMFLAGS=-I m4

# Testing - https://www.gnu.org/software/automake/manual/html_node/Scripts_002dbased-Testsuites.html#Scripts_002dbased-Testsuites
# TESTS=test1.sh
