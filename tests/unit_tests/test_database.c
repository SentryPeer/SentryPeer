/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only  */
/* Copyright (c) 2021 Gavin Henry <ghenry@sentrypeer.org> */
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

#include "test_database.h"
#include "../../src/database.h"

#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>

#define TEST_DB_FILE "test_sentrypeer.db"

void test_open_add_close_sqlite_db(void **state)
{
	(void)state; /* unused */

	const char schema_check[] = "PRAGMA user_version;";
	const char create_table_sql[] =
		"CREATE TABLE IF NOT EXISTS honey "
		"("
		"   honey_id INTEGER PRIMARY KEY,"
		"   event_timestamp TEXT,"
		"   event_uuid TEXT,"
		"   collected_method TEXT,"
		"   source_ip TEXT,"
		"   called_number TEXT,"
		"   transport_type TEXT,"
		"   method TEXT,"
		"   user_agent TEXT,"
		"   sip_message TEXT,"
		"   created_by_node_id TEXT,"
		"   created_at DATETIME DEFAULT(STRFTIME('%Y-%m-%d %H:%M:%f', 'NOW'))"
		");";

	const char insert_bad_actor[] =
		"INSERT INTO honey (event_timestamp,"
		"   event_uuid, collected_method, source_ip,"
		"   called_number, transport_type, method,"
		"   user_agent, sip_message, created_by_node_id) "
		"VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?);";

	sqlite3 *db;
	sqlite3_stmt *insert_bad_actor_stmt;
	// http://man.hubwiz.com/docset/SQLite.docset/Contents/Resources/Documents/sqlite/errlog.html
	sqlite3_config(SQLITE_CONFIG_LOG, db_error_log_callback, NULL);

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
	assert_int_equal(
		sqlite3_bind_text(insert_bad_actor_stmt, 2,
				  "f1f83c62-4fd9-11ec-a6bb-d05099894ba6", -1,
				  SQLITE_STATIC),
		SQLITE_OK);
	assert_int_equal(sqlite3_bind_text(insert_bad_actor_stmt, 3, "passive",
					   -1, SQLITE_STATIC),
			 SQLITE_OK);
	assert_int_equal(sqlite3_bind_text(insert_bad_actor_stmt, 4,
					   "104.149.141.214", -1,
					   SQLITE_STATIC),
			 SQLITE_OK);
	assert_int_equal(sqlite3_bind_text(insert_bad_actor_stmt, 5, "100", -1,
					   SQLITE_STATIC),
			 SQLITE_OK);
	assert_int_equal(sqlite3_bind_text(insert_bad_actor_stmt, 6, "UDP", -1,
					   SQLITE_STATIC),
			 SQLITE_OK);
	assert_int_equal(sqlite3_bind_text(insert_bad_actor_stmt, 7, "INVITE",
					   -1, SQLITE_STATIC),
			 SQLITE_OK);
	assert_int_equal(sqlite3_bind_text(insert_bad_actor_stmt, 8,
					   "friendly-scanner", -1,
					   SQLITE_STATIC),
			 SQLITE_OK);
	assert_int_equal(
		sqlite3_bind_text(
			insert_bad_actor_stmt, 9,
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

	assert_int_equal(
		sqlite3_bind_text(insert_bad_actor_stmt, 10,
				  "1f45cc1c-4fd4-11ec-89f0-d05099894ba6", -1,
				  SQLITE_STATIC),
		SQLITE_OK);

	fprintf(stderr,
		"Bound bad actor values for insert at line number %d in file %s\n",
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

// TODO: Fill out this
void test_db_insert_bad_actor(void **state)
{
	(void)state; /* unused */

	char test_source_ip[] = "127.0.0.1";
	char test_transport_type[] = "UDP";
	char test_collected_method[] = "passive";
	bad_actor *bad_actor_event =
		bad_actor_new(0, test_source_ip, 0, 0, test_transport_type, 0,
			      test_collected_method, 0);
	fprintf(stderr,
		"New bad actor event created at line number %d in file %s\n",
		__LINE__ - 1, __FILE__);
	assert_non_null(bad_actor_event);

	struct sentrypeer_config config;
	config.debug_mode = true;
	fprintf(stderr, "Debug mode set to true at line number %d in file %s\n",
		__LINE__ - 1, __FILE__);

	assert_int_equal(db_insert_bad_actor(bad_actor_event, &config),
			 EXIT_SUCCESS);

	// TODO: DB_FILE needs to be set via ENV or cli. In database.h for now
	// We'll be doing https://12factor.net/ anyway
	assert_int_equal(remove(DB_FILE), EXIT_SUCCESS);
	fprintf(stderr, "Removed database successfully.\n");
}
