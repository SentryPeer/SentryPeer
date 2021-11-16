/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only */
/* Copyright (c) 2021 Gavin Henry <ghenry@sentrypeer.org> */

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <stdint.h>
#include <cmocka.h>

#include "test_database.h"

#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>

#define TEST_DB_FILE "tests/sentrypeer.db"

void open_add_close_sqlite_db(void **state)
{

	const char *schema_check = "PRAGMA user_version;";
	const char *create_table_sql = "CREATE TABLE IF NOT EXISTS honey ("
				       "   honey_id INTEGER PRIMARY KEY,"
				       "   event_timestamp TEXT,"
				       "   source_ip TEXT,"
				       "   called_number TEXT,"
				       "   transport_type TEXT,"
				       "   sip_method TEXT,"
				       "   sip_user_agent TEXT,"
				       "   sip_message TEXT"
				       ");";

	sqlite3 *db;
	assert_int_equal(sqlite3_open(TEST_DB_FILE, &db), SQLITE_OK);
	fprintf(stderr,
		"Opened database successfully at line number %d in file %s\n",
		__LINE__ - 1, __FILE__);

	assert_int_equal(sqlite3_exec(db, schema_check, NULL, NULL, NULL),
			 SQLITE_OK);

	assert_int_equal(sqlite3_exec(db, create_table_sql, NULL, NULL, NULL),
			 SQLITE_OK);
	fprintf(stderr,
		"Created table successfully using: %s at line number %d in file %s\n",
		create_table_sql, __LINE__ - 1, __FILE__);

	assert_int_equal(sqlite3_close(db), SQLITE_OK);
	fprintf(stderr, "Closed database successfully.\n");

	assert_int_equal(remove(TEST_DB_FILE), EXIT_SUCCESS);
	fprintf(stderr, "Removed database successfully.\n");
}
