/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only */
/* Copyright (c) 2021 Gavin Henry <ghenry@sentrypeer.org> */

// https://git.cryptomilk.org/projects/cmocka.git/tree/example/simple_test.c

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <stdint.h>
#include <cmocka.h>

#include "test_utils.h"
#include "test_bad_actor.h"
#include "test_database.h"
#include "test_web_api.h"

/* A test case that does nothing and succeeds. */
static void null_test_success(void **state)
{
	(void)state; /* unused */
}

int main(void)
{
	const struct CMUnitTest tests[] = {
		cmocka_unit_test(null_test_success),
		cmocka_unit_test(test_utils),
		cmocka_unit_test(test_bad_actor),
		cmocka_unit_test(test_open_add_close_sqlite_db),
		cmocka_unit_test(test_db_insert_bad_actor),
		cmocka_unit_test(test_web_api_libmicrohttpd_get),
	};

	return cmocka_run_group_tests(tests, NULL, NULL);
}
