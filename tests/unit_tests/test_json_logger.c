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

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <stdint.h>
#include <cmocka.h>

#include "../../src/json_logger.h"
#include "test_bad_actor.h"

void test_json_logger(void **state)
{
	sentrypeer_config *config = *state;
	assert_non_null(config);

	bad_actor *bad_actor_to_log = test_bad_actor_event_new();

	assert_int_equal(json_log_bad_actor(config, bad_actor_to_log),
			 EXIT_SUCCESS);

	bad_actor_destroy(&bad_actor_to_log);
	assert_null(bad_actor_to_log);

	assert_int_equal(remove(config->json_log_file), EXIT_SUCCESS);
}
