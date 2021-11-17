/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only */
/* Copyright (c) 2021 Gavin Henry <ghenry@sentrypeer.org> */

#include "database.h"

#include <stdio.h>

void error_log_callback(void *arg, int err_code, const char *msg)
{
	fprintf(stderr, "Database error (%d): %s\n", err_code, msg);
}
