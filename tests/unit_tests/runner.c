/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only */
/* Copyright (c) 2021 Gavin Henry <ghenry@sentrypeer.org> */

// https://git.cryptomilk.org/projects/cmocka.git/tree/example/simple_test.c

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <stdint.h>
#include <cmocka.h>

#include <sqlite3.h>

/* A test case that does nothing and succeeds. */
static void null_test_success(void **state)
{
	(void)state; /* unused */
}

static void open_add_close_sqlite_db(void **state)
{
	sqlite3 *db;
	int open_rc = sqlite3_open("sentrypeer.db", &db);
	assert_int_equal(open_rc, SQLITE_OK);

	int close_rc = sqlite3_close(db);
	assert_int_equal(close_rc, SQLITE_OK);
}

int main(void)
{
	const struct CMUnitTest tests[] = {
		cmocka_unit_test(null_test_success),
		cmocka_unit_test(open_add_close_sqlite_db),
	};

	return cmocka_run_group_tests(tests, NULL, NULL);
}
