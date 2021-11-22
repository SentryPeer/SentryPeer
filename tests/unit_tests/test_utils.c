/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only */
/* Copyright (c) 2021 Gavin Henry <ghenry@sentrypeer.org> */

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <stdint.h>
#include <cmocka.h>

#include "test_utils.h"
#include "../../src/utils.h"

void test_utils(void **state)
{
	(void)state; /* unused */

	char string_to_copy[] = "I'm a string that will be copied!";
	assert_non_null(string_to_copy);

	char *string_copied = util_duplicate_string(string_to_copy);
	assert_non_null(string_copied);
	assert_string_equal(string_to_copy, string_copied);
}