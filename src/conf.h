/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only */
/* Copyright (c) 2021 Gavin Henry <ghenry@sentrypeer.org> */

#ifndef SENTRYPEER_CONFIG_H
#define SENTRYPEER_CONFIG_H 1

#include <getopt.h>
#include <stdbool.h>

struct sentrypeer_config {
	bool verbose_mode;
	bool debug_mode;
	bool daemon_mode;
	bool foreground_mode;
	bool sip_mode;
	bool bgp_mode;
	bool api_mode;
	bool web_gui_mode;
};

int process_cli(struct sentrypeer_config *config, int argc, char **argv);

#endif // SENTRYPEER_CONFIG_H
