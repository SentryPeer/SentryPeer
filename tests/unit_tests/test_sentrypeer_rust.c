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

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <stdint.h>
#include <cmocka.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "test_sentrypeer_rust.h"
#include "test_bad_actor.h"

// So it's defined here, and we don't use the return_exit_status Rust version
int32_t return_exit_status_c(bool success)
{
	if (success) {
		return EXIT_SUCCESS;
	} else {
		return EXIT_FAILURE;
	}
}

void test_sentrypeer_rust(void **state)
{
	(void)state; /* unused */

	fprintf(stderr, "Greetings from Rust at line number %d in file %s\n",
		__LINE__ - 1, __FILE__);
	display_rust();

	assert_int_equal(return_exit_status(true), EXIT_SUCCESS);
	assert_int_equal(return_exit_status(false), EXIT_FAILURE);
	assert_int_equal(callback_from_c(return_exit_status_c, true),
			 EXIT_SUCCESS);

	char *s = return_string();
	assert_string_equal(s, "Greetings from Rust");
	free_string(s);
	assert_non_null(s);

	bad_actor *ba = return_bad_actor_new("INVITE blah", "127.0.0.1",
					     "127.0.0.1", "123456", "INVITE",
					     "UDP", "SIPp", "passive", "12345");
	assert_non_null(ba);

	fprintf(stderr, "bad_actor: %s %s %s %s %s %s %s %s %s %s %s\n",
		ba->collected_method, ba->created_by_node_id, ba->sip_message,
		ba->source_ip, ba->destination_ip, ba->called_number,
		ba->method, ba->transport_type, ba->user_agent, ba->seen_last,
		ba->seen_count);

	assert_string_equal(ba->sip_message, "INVITE blah");
	assert_string_equal(ba->source_ip, "127.0.0.1");
	assert_string_equal(ba->destination_ip, "127.0.0.1");
	assert_string_equal(ba->called_number, "123456");
	assert_string_equal(ba->method, "INVITE");
	assert_string_equal(ba->transport_type, "UDP");
	assert_string_equal(ba->user_agent, "SIPp");
	assert_string_equal(ba->collected_method, "passive");
	assert_string_equal(ba->created_by_node_id, "12345");
	assert_null(ba->seen_last);
	assert_null(ba->seen_count);
}
