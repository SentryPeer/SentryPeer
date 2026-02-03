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

#include "test_utils.h"
#include "../../src/utils.h"
#include "../../src/conf.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void test_utils(void **state)
{
	(void)state; /* unused */

	// String utils
	char string_to_duplicate[] = "I'm a string that will be copied!";
	assert_non_null(string_to_duplicate);

	// util_duplicate_string
	char *string_copied = util_duplicate_string(string_to_duplicate);
	assert_non_null(string_copied);
	assert_string_equal(string_to_duplicate, string_copied);
	free(string_copied);
	fprintf(stderr,
		"Freed string_copied successfully at line number %d in file %s\n",
		__LINE__ - 1, __FILE__);

	// Modern C by Manning, Takeaway 6.19
	// "6.19 Initialization or assignment with 0 makes a pointer null."
	string_copied = 0;
	assert_null(string_copied);

	// util_copy_string
	char *destination_string =
		calloc(SENTRYPEER_PATH_MAX + 1, sizeof(char));
	assert_non_null(destination_string);
	destination_string = util_copy_string(
		destination_string, string_to_duplicate, SENTRYPEER_PATH_MAX);
	assert_non_null(destination_string);
	assert_null(destination_string[strlen(destination_string)]);
	assert_string_equal(destination_string, string_to_duplicate);

	free(destination_string);
	fprintf(stderr,
		"Freed destination_string successfully at line number %d in file %s\n",
		__LINE__ - 1, __FILE__);
	destination_string = 0;
	assert_null(destination_string);

	// uuid utils
	char uuid_string[UTILS_UUID_STRING_LEN];
	util_uuid_generate_string(uuid_string);
	assert_non_null(uuid_string);
	assert_string_not_equal(uuid_string, "");
	assert_string_not_equal(uuid_string,
				"00000000-0000-0000-0000-000000000000");

	// valid_ip_address_format
	assert_int_equal(valid_ip_address_format("127.0.0.1"), EXIT_SUCCESS);
	assert_int_equal(valid_ip_address_format(
				 "2001:0db8:85a3:0000:0000:8a2e:0370:7334"),
			 EXIT_SUCCESS);
	assert_int_equal(valid_ip_address_format("NOT_AN_IP_ADDRESS"),
			 EXIT_FAILURE);

	// JSON utils
	json_t *json = json_object();
	assert_non_null(json);

	json_object_set_new(json, "key", json_string("value"));

	assert_false(is_valid_json_key(NULL, NULL));
	assert_false(is_valid_json_key(json, NULL));
	assert_false(is_valid_json_key(NULL, ""));
	assert_false(is_valid_json_key(json, ""));
	assert_false(is_valid_json_key(json, "key_does_not_exist"));
	assert_true(is_valid_json_key(json, "key"));

	json_decref(json);

	// UUID utils
	char uuid_valid[] = "00000000-0000-0000-0000-000000000000";
	char uuid_invalid[] = "0000000000-054564654654000-0000-000000000000123";
	assert_true(is_valid_uuid(uuid_valid));
	assert_false(is_valid_uuid(uuid_invalid));

	// max_int
	assert_int_equal(max_int(1, 2), 2);
}
