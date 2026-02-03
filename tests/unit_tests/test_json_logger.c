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

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <stdint.h>
#include <cmocka.h>

#include "../../src/json_logger.h"
#include "test_bad_actor.h"

#if HAVE_RUST != 0
#include "../../src/sentrypeer_rust.h"
#endif

// cppcheck-suppress constParameter
void test_json_logger(void **state)
{
	sentrypeer_config *config = *state;
	assert_non_null(config);

	// Set up
	bad_actor *bad_actor_event = test_bad_actor_event_new();
	assert_non_null(bad_actor_event);

	if (config->new_mode == true) {
#if HAVE_RUST != 0

		// Conversion tests
		char *json_string = bad_actor_to_json_rs(config, bad_actor_event);
		assert_non_null(json_string);

		bad_actor *bad_actor_from_json =
			json_to_bad_actor_rs(config, json_string);
		assert_non_null(bad_actor_from_json);

		// Log JSON via Rust
		assert_int_equal(json_log_bad_actor_rs(config, bad_actor_event),
				 EXIT_SUCCESS);

		// Clean up
		bad_actor_destroy(&bad_actor_event);
		assert_null(bad_actor_event);

		bad_actor_destroy(&bad_actor_from_json);
		assert_null(bad_actor_from_json);

		free(json_string);
#endif
	} else {
		// Log JSON to the log file
		assert_int_equal(json_log_bad_actor(config, bad_actor_event),
				 EXIT_SUCCESS);

		// Conversion tests
		char *json_string = bad_actor_to_json(config, bad_actor_event);
		assert_non_null(json_string);

		bad_actor *bad_actor_from_json =
			json_to_bad_actor(config, json_string);
		assert_non_null(bad_actor_from_json);

		// Clean up
		bad_actor_destroy(&bad_actor_event);
		assert_null(bad_actor_event);
		bad_actor_destroy(&bad_actor_from_json);
		assert_null(bad_actor_from_json);
		free(json_string);
	}

	assert_int_equal(remove(config->json_log_file), EXIT_SUCCESS);
}
