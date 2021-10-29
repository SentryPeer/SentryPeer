/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only */
/* Copyright (c) 2021 Gavin Henry <ghenry@sentrypeer.org> */

#ifndef SENTRYPEER_CONFIG_H
#define SENTRYPEER_CONFIG_H

#include <getopt.h>

void print_usage(void);
void print_version(void);
void process_cli(int argc, char **argv);

#endif // SENTRYPEER_CONFIG_H
