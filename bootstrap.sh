#!/bin/sh

mkdir -p m4
autoreconf --verbose --force --install || exit 1
