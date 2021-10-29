/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only  */
/* Copyright (c) 2021 Gavin Henry <ghenry@sentrypeer.org> */

#include "sentrypeer.h"

int main(int argc, char **argv)
{
	if (argc <= 1) {
		print_usage();
		exit(EXIT_SUCCESS);
	}

	process_cli(argc, argv);

	return EXIT_SUCCESS;
}