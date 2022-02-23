/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only  */
/* Copyright (c) 2021 - 2022 Gavin Henry <ghenry@sentrypeer.org> */
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

#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <assert.h>
#include <signal.h>
#include <unistd.h>

// Produced by autoconf and cmake (manually by me)
#include "config.h"

#include "signal_handler.h"
#include "conf.h"
#include "sip_daemon.h"
#include "http_daemon.h"

#ifndef DISABLE_ZYRE // This is a compile-time flag
#include "peer_to_peer_lan.h"
#endif // DISABLE_ZYRE

volatile sig_atomic_t cleanup_flag = 0;

int main(int argc, char **argv)
{
	sentrypeer_config *config = sentrypeer_config_new();
	assert(config);

	signal_handler_init();

	if (process_cli(config, argc, argv) != EXIT_SUCCESS) {
		exit(EXIT_FAILURE);
	}

	if (config->syslog_mode) {
		openlog(PACKAGE_NAME, LOG_PID, LOG_USER);
	}

	if (config->debug_mode || config->verbose_mode) {
		fprintf(stderr, "Starting %s...\n", PACKAGE_NAME);
		if (config->syslog_mode) {
			syslog(LOG_ERR, "Starting %s...\n", PACKAGE_NAME);
		}
	}

	// Threaded, so start the HTTP daemon first
	if (config->api_mode && (http_daemon_init(config) != EXIT_SUCCESS)) {
		fprintf(stderr, "Failed to start %s server on port %d\n",
			"HTTP", HTTP_DAEMON_PORT);
		perror("http_daemon_init");
		if (config->syslog_mode) {
			syslog(LOG_ERR,
			       "Failed to start %s server on port %d\n", "HTTP",
			       HTTP_DAEMON_PORT);
		}
		exit(EXIT_FAILURE);
	}

	if (config->web_gui_mode &&
	    (config->debug_mode || config->verbose_mode)) {
		fprintf(stderr, "Web GUI mode enabled...\n");
	}

	if (sip_daemon_run(config) != EXIT_SUCCESS) {
		fprintf(stderr, "Failed to start %s server on port %s\n", "SIP",
			SIP_DAEMON_PORT);
		perror("sip_daemon_run");
		if (config->syslog_mode) {
			syslog(LOG_ERR,
			       "Failed to start %s server on port %s\n", "SIP",
			       SIP_DAEMON_PORT);
		}
		exit(EXIT_FAILURE);
	}

#ifndef DISABLE_ZYRE
	// TODO: Add p2p flag to config/cli
	if (peer_to_peer_lan_run(config) != EXIT_SUCCESS) {
		fprintf(stderr, "Failed to start peer to peer LAN.\n");
		perror("peer_to_peer_lan_run");
		if (config->syslog_mode) {
			syslog(LOG_ERR, "Failed to start peer to peer LAN\n");
		}
		exit(EXIT_FAILURE);
	}

	while (cleanup_flag == 0 && zsys_interrupted == 0) {
		sleep(1);
	}
#else
	while (cleanup_flag == 0) {
		sleep(1);
	}
#endif // DISABLE_ZYRE

	if (config->debug_mode || config->verbose_mode) {
		fprintf(stderr, "Stopping %s...\n", PACKAGE_NAME);
		if (config->syslog_mode) {
			syslog(LOG_ERR, "Stopping %s...\n", PACKAGE_NAME);
		}
	}

	if (config->api_mode && (http_daemon_stop(config) != EXIT_SUCCESS)) {
		fprintf(stderr, "Issue cleanly stopping http_daemon.\n");
	}

	if (sip_daemon_stop(config) != EXIT_SUCCESS) {
		fprintf(stderr, "Issue cleanly stopping sip_daemon.\n");
	}

#ifndef DISABLE_ZYRE // This is a compile-time flag

	// TODO: Add p2p flag to config/cli
	if (peer_to_peer_lan_stop(config) != EXIT_SUCCESS) {
		fprintf(stderr, "Issue cleanly stopping peer_to_peer_lan.\n");
	}
#endif // DISABLE_ZYRE

	if (config->debug_mode || config->verbose_mode) {
		fprintf(stderr, "Stopped %s\n", PACKAGE_NAME);
		if (config->syslog_mode) {
			syslog(LOG_ERR, "Stopped %s\n", PACKAGE_NAME);
		}
	}
	sentrypeer_config_destroy(&config);

	return EXIT_SUCCESS;
}
