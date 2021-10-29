/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only  */
/* Copyright (c) 2021 Gavin Henry <ghenry@sentrypeer.org> */

#include "sentrypeer.h"

int main(int argc, char **argv)
{
	int rc;
	struct sentrypeer_config config;

	rc = process_cli(&config, argc, argv);
	if (rc != EXIT_SUCCESS) {
		exit(rc);
	}

	if (config.debug_mode || config.verbose_mode) {
		fprintf(stderr, "Starting %s\n", PACKAGE_NAME);
	}

	fprintf(stderr, "%s is about to do something?\n", PACKAGE_NAME);
	return EXIT_SUCCESS;
}
