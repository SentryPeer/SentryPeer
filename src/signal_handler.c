/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only  */
/* Copyright (c) 2021 - 2025 Gavin Henry <ghenry@sentrypeer.org> */
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

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>

#include "signal_handler.h"

/* Signal handler (used to intercept CTRL+C and SIGTERM) */
static void signal_handler(int signo)
{
	// We don't care about the signal type at the moment
	// Just set the flag, so we can exit cleanly
	cleanup_flag = signo;
}

/* Termination handler (atexit) */
static void termination_handler(void)
{
	cleanup_flag = 1;
	closelog();
}

int signal_handler_init(void)
{
	// Handle SIGINT (CTRL-C)
	if (signal(SIGINT, signal_handler) == SIG_ERR) {
		fprintf(stderr, "Cannot handle SIGINT!\n");
		exit(EXIT_FAILURE);
	}

	// Handle SIGTERM (from service managers) */
	if (signal(SIGTERM, signal_handler) == SIG_ERR) {
		fprintf(stderr, "Cannot handle SIGTERM!\n");
		exit(EXIT_FAILURE);
	}

	// Reset SIGPROF's behavior to the default
	if (signal(SIGPROF, SIG_DFL) == SIG_ERR) {
		fprintf(stderr, "Cannot reset SIGPROF!\n");
		exit(EXIT_FAILURE);
	}

	// Ignore SIGHUP
	if (signal(SIGHUP, SIG_IGN) == SIG_ERR) {
		fprintf(stderr, "Cannot ignore SIGHUP!\n");
		exit(EXIT_FAILURE);
	}
	// Last thing that happens before termination
	if (atexit(termination_handler) != EXIT_SUCCESS) {
		fprintf(stderr, "Cannot register termination handler!\n");
		exit(EXIT_FAILURE);
	}

	return EXIT_SUCCESS;
}
