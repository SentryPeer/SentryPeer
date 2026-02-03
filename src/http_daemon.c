/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only  */
/* Copyright (c) 2021 - 2026 Gavin Henry <ghenry@sentrypeer.org> */
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

#include "conf.h"
#include "http_daemon.h"
#include "http_routes.h"

#include <stdio.h>
#include <stdlib.h>

#include <microhttpd.h>

int http_daemon_init(sentrypeer_config *config)
{
	if (config->debug_mode || config->verbose_mode) {
		fprintf(stderr, "API mode enabled, starting http daemon...\n");
	}

	struct MHD_Daemon *daemon;

	daemon = MHD_start_daemon(MHD_USE_THREAD_PER_CONNECTION,
				  HTTP_DAEMON_PORT, NULL, NULL, &route_handler,
				  config, MHD_OPTION_END);
	if (daemon == NULL) {
		return EXIT_FAILURE;
	}
	config->http_daemon = daemon;

	return EXIT_SUCCESS;
}

int http_daemon_stop(sentrypeer_config *config)
{
	if (config->debug_mode || config->verbose_mode) {
		fprintf(stderr, "Stopping http daemon...\n");
	}

	MHD_stop_daemon(config->http_daemon);

	return EXIT_SUCCESS;
}
