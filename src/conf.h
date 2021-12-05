/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only  */
/* Copyright (c) 2021 Gavin Henry <ghenry@sentrypeer.org> */
/*
   _____            _              _____
  / ____|          | |            |  __ \
 | (___   ___ _ __ | |_ _ __ _   _| |__) |__  ___ _ __
  \___ \ / _ \ '_ \| __| '__| | | |  ___/ _ \/ _ \ '__|
  ____) |  __/ | | | |_| |  | |_| | |  |  __/  __/ |
 |_____/ \___|_| |_|\__|_|   \__, |_|   \___|\___|_|
                              __/ |
                             |___/
*/

#ifndef SENTRYPEER_CONFIG_H
#define SENTRYPEER_CONFIG_H 1

#include <getopt.h>
#include <stdbool.h>

struct sentrypeer_config {
	bool syslog_mode;
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
