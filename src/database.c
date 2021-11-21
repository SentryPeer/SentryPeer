/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only */
/* Copyright (c) 2021 Gavin Henry <ghenry@sentrypeer.org> */

#include <stdio.h>

#include "database.h"
#include <stdlib.h>

void error_log_callback(int err_code, const char *msg)
{
	fprintf(stderr, "Database error (%d): %s\n", err_code, msg);
}

int db_insert_bad_actor(bad_actor *bad_actor_event,
			struct sentrypeer_config const *config)
{
	return EXIT_SUCCESS;
}