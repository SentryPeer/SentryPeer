/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only */
/* Copyright (c) 2021 Gavin Henry <ghenry@sentrypeer.org> */

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <stdint.h>
#include <cmocka.h>

#include "test_database.h"
#include "../../src/database.h"

#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>

#define TEST_DB_FILE "./sentrypeer.db"

void open_add_close_sqlite_db(void **state)
{
	const char *schema_check = "PRAGMA user_version;";
	const char *create_table_sql =
		"CREATE TABLE IF NOT EXISTS honey "
		"("
		"   honey_id INTEGER PRIMARY KEY,"
		"   event_timestamp TEXT,"
		"   source_ip TEXT,"
		"   called_number TEXT,"
		"   transport_type TEXT,"
		"   method TEXT,"
		"   user_agent TEXT,"
		"   sip_message TEXT,"
		"   created_at DATETIME DEFAULT(STRFTIME('%Y-%m-%d %H:%M:%f', 'NOW'))"
		");";

	const char *insert_bad_actor = "INSERT INTO honey (event_timestamp,"
				       "   source_ip, called_number,"
				       "   transport_type, method,"
				       "   user_agent, sip_message) "
				       "VALUES (?, ?, ?, ?, ?, ?, ?)";

	sqlite3 *db;
	sqlite3_stmt *insert_bad_actor_stmt;
	// http://man.hubwiz.com/docset/SQLite.docset/Contents/Resources/Documents/sqlite/errlog.html
	sqlite3_config(SQLITE_CONFIG_LOG, error_log_callback, NULL);

	assert_int_equal(sqlite3_open(TEST_DB_FILE, &db), SQLITE_OK);
	fprintf(stderr,
		"Opened database successfully at line number %d in file %s\n",
		__LINE__ - 1, __FILE__);

	assert_int_equal(sqlite3_exec(db, schema_check, NULL, NULL, NULL),
			 SQLITE_OK);
	fprintf(stderr, "Checked schema at line number %d in file %s\n",
		__LINE__ - 1, __FILE__);

	assert_int_equal(sqlite3_exec(db, create_table_sql, NULL, NULL, NULL),
			 SQLITE_OK);
	fprintf(stderr,
		"Created table successfully using: %s at line number %d in file %s\n",
		create_table_sql, __LINE__ - 1, __FILE__);

	assert_int_equal(sqlite3_prepare_v2(db, insert_bad_actor, -1,
					    &insert_bad_actor_stmt, NULL),
			 SQLITE_OK);
	fprintf(stderr,
		"Prepared insert bad actor statement at line number %d in file %s\n",
		__LINE__ - 1, __FILE__);

	assert_int_equal(sqlite3_bind_text(insert_bad_actor_stmt, 1,
					   "2020-11-18 20:13:17.349557105", -1,
					   SQLITE_STATIC),
			 SQLITE_OK);
	assert_int_equal(sqlite3_bind_text(insert_bad_actor_stmt, 2,
					   "104.149.141.214", -1,
					   SQLITE_STATIC),
			 SQLITE_OK);
	assert_int_equal(sqlite3_bind_text(insert_bad_actor_stmt, 3, "100", -1,
					   SQLITE_STATIC),
			 SQLITE_OK);
	assert_int_equal(sqlite3_bind_text(insert_bad_actor_stmt, 4, "UDP", -1,
					   SQLITE_STATIC),
			 SQLITE_OK);
	assert_int_equal(sqlite3_bind_text(insert_bad_actor_stmt, 5, "INVITE",
					   -1, SQLITE_STATIC),
			 SQLITE_OK);
	assert_int_equal(sqlite3_bind_text(insert_bad_actor_stmt, 6,
					   "friendly-scanner", -1,
					   SQLITE_STATIC),
			 SQLITE_OK);
	assert_int_equal(
		sqlite3_bind_text(
			insert_bad_actor_stmt, 7,
			"OPTIONS sip:100@XXX.XXX.XXX.XXX SIP/2.0\n"
			"Via: SIP/2.0/UDP 23.148.145.71:5084;branch=z9hG4bK-3054909403;rport\n"
			"From: \"sipvicious\" <sip:100@1.1.1.1>;tag=6434396633623535313363340133343333313138393833\n"
			"To: \"sipvicious\" <sip:100@1.1.1.1>\n"
			"Call-ID: 711444933874895842969934\n"
			"CSeq: 1 OPTIONS\n"
			"Contact: <sip:100@23.148.145.71:5084>\n"
			"Accept: application/sdp\n"
			"User-agent: friendly-scanner\n"
			"Max-forwards: 70\n"
			"Content-Length: 0",
			-1, SQLITE_STATIC),
		SQLITE_OK);

	fprintf(stderr,
		"Bound bad actor values fpr insert at line number %d in file %s\n",
		__LINE__ - 1, __FILE__);

	assert_int_equal(sqlite3_step(insert_bad_actor_stmt), SQLITE_DONE);
	fprintf(stderr,
		"Evaluated bad actor insert at line number %d in file %s\n",
		__LINE__ - 1, __FILE__);

	assert_int_equal(sqlite3_finalize(insert_bad_actor_stmt), SQLITE_OK);
	fprintf(stderr,
		"Finalized insert bad actor statement at line number %d in file %s\n",
		__LINE__ - 1, __FILE__);

	assert_int_equal(sqlite3_close(db), SQLITE_OK);
	fprintf(stderr, "Closed database successfully.\n");

	assert_int_equal(remove(TEST_DB_FILE), EXIT_SUCCESS);
	fprintf(stderr, "Removed database successfully.\n");
}
