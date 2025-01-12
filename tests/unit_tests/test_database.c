/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only  */
/* Copyright (c) 2021 - 2025 Gavin Henry <ghenry@sentrypeer.org> */
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
#include <string.h>

#define BAD_ACTOR_EVENT_UUID "2ceba0a8-3ac1-426f-9d45-8ecc3f00c21e"
#define NODE_ID "1f45cc1c-4fd4-11ec-89f0-d05099894ba6"
#define BAD_ACTOR_SOURCE_IP "104.149.141.214"

// https://api.cmocka.org/group__cmocka__exec.html#ga7c62fd0acf2235ce98268c28ee262a57
int test_setup_sqlite_db(void **state)
{
	sentrypeer_config *config = sentrypeer_config_new();
	config->debug_mode = true;
	assert_non_null(config);
	strncpy(config->db_file, TEST_DB_FILE, SENTRYPEER_PATH_MAX);
	assert_non_null(config->db_file);

	*state = config;
	assert_non_null(state);

	sqlite3 *db;
	assert_int_equal(sqlite3_open(config->db_file, &db), SQLITE_OK);
	fprintf(stderr,
		"Opened database successfully at line number %d in file %s\n",
		__LINE__ - 1, __FILE__);

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

	const char create_source_ip_index[] =
		"CREATE INDEX IF NOT EXISTS source_ip_index ON honey (source_ip);";

	const char create_called_number_index[] =
		"CREATE INDEX IF NOT EXISTS called_number_index ON honey (called_number);";

	const char create_event_uuid_index[] =
		"CREATE INDEX IF NOT EXISTS event_uuid_index ON honey (event_uuid);";

	const char insert_bad_actor[] =
		"INSERT INTO honey (event_timestamp,"
		"   event_uuid, collected_method, source_ip,"
		"   called_number, transport_type, method,"
		"   user_agent, sip_message, created_by_node_id) "
		"VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?);";

	sqlite3_stmt *insert_bad_actor_stmt;

	assert_int_equal(sqlite3_exec(db, schema_check, NULL, NULL, NULL),
			 SQLITE_OK);
	fprintf(stderr, "Checked schema at line number %d in file %s\n",
		__LINE__ - 1, __FILE__);

	assert_int_equal(sqlite3_exec(db, create_table_sql, NULL, NULL, NULL),
			 SQLITE_OK);
	fprintf(stderr,
		"Created table successfully using: %s at line number %d in file %s\n",
		create_table_sql, __LINE__ - 1, __FILE__);

	assert_int_equal(sqlite3_exec(db, create_source_ip_index, NULL, NULL,
				      NULL),
			 SQLITE_OK);
	fprintf(stderr,
		"Created index on source_ip successfully using: %s at line number %d in file %s\n",
		create_table_sql, __LINE__ - 1, __FILE__);

	assert_int_equal(sqlite3_exec(db, create_called_number_index, NULL,
				      NULL, NULL),
			 SQLITE_OK);
	fprintf(stderr,
		"Created index on called_number successfully using: %s at line number %d in file %s\n",
		create_table_sql, __LINE__ - 1, __FILE__);

	assert_int_equal(sqlite3_exec(db, create_event_uuid_index, NULL, NULL,
				      NULL),
			 SQLITE_OK);
	fprintf(stderr,
		"Created index on event_uuid successfully using: %s at line number %d in file %s\n",
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
					   BAD_ACTOR_EVENT_UUID, -1,
					   SQLITE_STATIC),
			 SQLITE_OK);
	assert_int_equal(sqlite3_bind_text(insert_bad_actor_stmt, 3, "passive",
					   -1, SQLITE_STATIC),
			 SQLITE_OK);
	assert_int_equal(sqlite3_bind_text(insert_bad_actor_stmt, 4,
					   BAD_ACTOR_SOURCE_IP, -1,
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

	assert_int_equal(sqlite3_bind_text(insert_bad_actor_stmt, 10, NODE_ID,
					   -1, SQLITE_STATIC),
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

	sqlite3_close(db);
	fprintf(stderr,
		"Closed database successfully at line number %d in file %s\n",
		__LINE__ - 1, __FILE__);

	return EXIT_SUCCESS;
}

// cppcheck-suppress constParameter
int test_teardown_sqlite_db(void **state)
{
	sentrypeer_config *config = *state;
	assert_non_null(config);

	fprintf(stderr, "Removing database: %s\n", config->db_file);
	assert_int_equal(remove(config->db_file), EXIT_SUCCESS);
	fprintf(stderr, "Removed database successfully.\n");

	sentrypeer_config_destroy(&config);
	assert_null(config);
	return EXIT_SUCCESS;
}

// cppcheck-suppress constParameter
void test_open_select_close_sqlite_db(void **state)
{
	sentrypeer_config *config = *state;
	assert_non_null(config);

	sqlite3 *db;
	sqlite3_stmt *select_bad_actor_stmt;

	assert_int_equal(sqlite3_open(config->db_file, &db), SQLITE_OK);
	fprintf(stderr,
		"Opened database successfully at line number %d in file %s\n",
		__LINE__ - 1, __FILE__);

	char select_bad_actor_by_uuid[] =
		"SELECT source_ip FROM honey WHERE event_uuid = ?";
	assert_int_equal(sqlite3_prepare_v2(db, select_bad_actor_by_uuid, -1,
					    &select_bad_actor_stmt, NULL),
			 SQLITE_OK);

	assert_int_equal(sqlite3_bind_text(select_bad_actor_stmt, 1,
					   BAD_ACTOR_EVENT_UUID, -1,
					   SQLITE_STATIC),
			 SQLITE_OK);

	assert_int_equal(sqlite3_step(select_bad_actor_stmt), SQLITE_ROW);
	assert_string_equal((const char*)sqlite3_column_text(select_bad_actor_stmt, 0),
			    BAD_ACTOR_SOURCE_IP);

	assert_int_equal(sqlite3_finalize(select_bad_actor_stmt), SQLITE_OK);
	fprintf(stderr,
		"Finalized select bad actor statement at line number %d in file %s\n",
		__LINE__ - 1, __FILE__);

	sqlite3_close(db);
	fprintf(stderr,
		"Closed database successfully at line number %d in file %s\n",
		__LINE__ - 1, __FILE__);
}

// cppcheck-suppress constParameter
void test_db_insert_bad_actor(void **state)
{
	sentrypeer_config *config = *state;
	assert_non_null(config);

	char test_source_ip[] = "127.0.0.1";
	char test_transport_type[] = "UDP";
	char test_collected_method[] = "passive";
	bad_actor *bad_actor_event =
		bad_actor_new(0, util_duplicate_string(test_source_ip), 0, 0, 0,
			      util_duplicate_string(test_transport_type), 0,
			      util_duplicate_string(test_collected_method),
			      config->node_id);
	fprintf(stderr,
		"New bad actor event created at line number %d in file %s\n",
		__LINE__ - 1, __FILE__);
	assert_non_null(bad_actor_event);

	fprintf(stderr, "Debug mode set to true at line number %d in file %s\n",
		__LINE__ - 1, __FILE__);

	assert_int_equal(db_insert_bad_actor(bad_actor_event, config),
			 EXIT_SUCCESS);
	bad_actor_destroy(&bad_actor_event);
	fprintf(stderr, "Freed bad_actor_event.\n");
	assert_null(bad_actor_event);
}

// cppcheck-suppress constParameter
void test_db_select_bad_actor(void **state)
{
	sentrypeer_config *config = *state;
	assert_non_null(config);

	// Find by IP address - should succeed
	bad_actor *bad_actor_found = 0;
	assert_int_equal(db_select_bad_actor_by_ip(BAD_ACTOR_SOURCE_IP,
						   &bad_actor_found, config),
			 EXIT_SUCCESS);
	assert_non_null(bad_actor_found);
	bad_actor_destroy(&bad_actor_found);
	assert_null(bad_actor_found);

	// Find by IP address - should fail
	bad_actor *bad_actor_not_found = 0;
	assert_int_equal(db_select_bad_actor_by_ip(
				 "127.0.0.1", &bad_actor_not_found, config),
			 EXIT_FAILURE);
	bad_actor_destroy(&bad_actor_not_found);
	assert_null(bad_actor_not_found);

	// Find by event_uuid
	assert_true(db_bad_actor_exists(BAD_ACTOR_EVENT_UUID, config));
	assert_false(db_bad_actor_exists("", config)); // invalid UUID
	assert_false(db_bad_actor_exists(NULL, config)); // invalid UUID
	assert_false(db_bad_actor_exists("85794cd7-f874-4346-a549-424898a0e224",
					 config)); // UUID does not exist
}

// cppcheck-suppress constParameter
void test_db_select_bad_actors(void **state)
{
	sentrypeer_config *config = *state;
	assert_non_null(config);

	bad_actor **bad_actors = 0;
	int64_t row_count = 0;
	assert_int_equal(db_select_bad_actors(&bad_actors, &row_count, config),
			 EXIT_SUCCESS);
	assert_non_null(bad_actors);
	assert_int_not_equal(row_count, 0);

	bad_actors_destroy(bad_actors, &row_count);
	fprintf(stderr, "Freed bad_actors.\n");
	free(bad_actors);
	bad_actors = 0;
	assert_null(bad_actors);
}
