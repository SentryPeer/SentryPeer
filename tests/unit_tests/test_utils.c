/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only */
/* Copyright (c) 2021 Gavin Henry <ghenry@sentrypeer.org> */

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <stdint.h>
#include <cmocka.h>

#include "test_utils.h"
#include "../../src/utils.h"
#include <uuid/uuid.h>

void test_utils(void **state)
{
	(void)state; /* unused */

	// String utils
	char string_to_copy[] = "I'm a string that will be copied!";
	assert_non_null(string_to_copy);

	char *string_copied = util_duplicate_string(string_to_copy);
	assert_non_null(string_copied);
	assert_string_equal(string_to_copy, string_copied);

	// uuid utils
	char uuid_string[UUID_STR_LEN];
	util_uuid_generate_string(uuid_string);
	assert_non_null(uuid_string);
	assert_string_not_equal(uuid_string, "");
	assert_string_not_equal(uuid_string, "00000000-0000-0000-0000-000000000000");
}