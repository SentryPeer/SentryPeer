/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only */
/* Copyright (c) 2021 Gavin Henry <ghenry@sentrypeer.org> */

#include "conf.h"
#include "sentrypeer.h"

void print_usage(void)
{
	fprintf(stderr, "Usage: %s [-h] [-V] [-v] [-d]\n", PACKAGE_NAME);
	fprintf(stderr, "\n");
	fprintf(stderr, "Options:\n");
	fprintf(stderr, "  -h,      Print this help.\n");
	fprintf(stderr, "  -V,      Print version.\n");
	fprintf(stderr, "  -v,      Enable all logging.\n");
	fprintf(stderr, "  -d,      Enable debug mode.\n");
	fprintf(stderr, "\n");
	fprintf(stderr,
		"Report bugs to https://github.com/SentryPeer/SentryPeer/issues\n");
	fprintf(stderr, "\nSee https://sentrypeer.org for more information.\n");
}

void print_version(void)
{
	fprintf(stderr, "This is %s version: %s, git rev: %s\n", PACKAGE_NAME,
		PACKAGE_VERSION, REVISION);
}

int process_cli(struct sentrypeer_config *config, int argc, char **argv)
{
	int cli_option;

	while ((cli_option = getopt(argc, argv, "hVvd")) != -1) {
		switch (cli_option) {
		case 'h':
			print_usage();
			return(EXIT_SUCCESS);
		case 'V':
			print_version();
			exit(EXIT_SUCCESS);
		case 'v':
			config->verbose_mode = true;
			break;
		case 'd':
			config->debug_mode = true;
			break;
		default:
			print_usage();
			return(EXIT_FAILURE);
		}
	}
	return(EXIT_SUCCESS);
}
