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

#define _GNU_SOURCE
#include <stdio.h>

#include "database.h"
#include <stdlib.h>
#include <assert.h>
#include <stdint.h>
#include <string.h>

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

void db_error_log_callback(int err_code, const char *msg)
{
	fprintf(stderr, "Database error (%d): %s\n", err_code, msg);
}

int db_set_error_log_callback(void)
{
	if (sqlite3_config(SQLITE_CONFIG_LOG, db_error_log_callback, NULL) !=
	    SQLITE_OK) {
		fprintf(stderr, "Failed to set error log callback\n");
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

int db_insert_bad_actor(bad_actor const *bad_actor_event,
			sentrypeer_config const *config)
{
	sqlite3 *db;
	sqlite3_stmt *insert_bad_actor_stmt;

	assert(config->db_file);

	if (config->debug_mode || config->verbose_mode) {
		fprintf(stderr, "SentryPeer db file location is: %s\n",
			config->db_file);
	}

	if (sqlite3_open(config->db_file, &db) != SQLITE_OK) {
		fprintf(stderr, "Failed to open database\n");
		sqlite3_close(db);
		return EXIT_FAILURE;
	}

	// TODO: Check if schema is needs to be updated
	if (sqlite3_exec(db, schema_check, NULL, NULL, NULL) != SQLITE_OK) {
		fprintf(stderr, "Failed to check schema\n");
		sqlite3_close(db);
		return EXIT_FAILURE;
	}

	if (sqlite3_exec(db, create_table_sql, NULL, NULL, NULL) != SQLITE_OK) {
		fprintf(stderr, "Failed to create table\n");
		sqlite3_close(db);
		return EXIT_FAILURE;
	}

	if (sqlite3_prepare_v2(db, insert_bad_actor, -1, &insert_bad_actor_stmt,
			       NULL) != SQLITE_OK) {
		fprintf(stderr, "Failed to prepare statement\n");
		sqlite3_close(db);
		return EXIT_FAILURE;
	}

	if (sqlite3_bind_text(insert_bad_actor_stmt, 1,
			      bad_actor_event->event_timestamp, -1,
			      SQLITE_STATIC) != SQLITE_OK) {
		fprintf(stderr, "Failed to bind event_timestamp\n");
		sqlite3_close(db);
		return EXIT_FAILURE;
	}

	if (sqlite3_bind_text(insert_bad_actor_stmt, 2,
			      bad_actor_event->event_uuid, -1,
			      SQLITE_STATIC) != SQLITE_OK) {
		fprintf(stderr, "Failed to bind event_uuid\n");
		sqlite3_close(db);
		return EXIT_FAILURE;
	}

	if (sqlite3_bind_text(insert_bad_actor_stmt, 3,
			      bad_actor_event->collected_method, -1,
			      SQLITE_STATIC) != SQLITE_OK) {
		fprintf(stderr, "Failed to bind collected_method\n");
		sqlite3_close(db);
		return EXIT_FAILURE;
	}

	if (sqlite3_bind_text(insert_bad_actor_stmt, 4,
			      bad_actor_event->source_ip, -1,
			      SQLITE_STATIC) != SQLITE_OK) {
		fprintf(stderr, "Failed to bind source_ip\n");
		sqlite3_close(db);
		return EXIT_FAILURE;
	}

	if (sqlite3_bind_text(insert_bad_actor_stmt, 5,
			      bad_actor_event->called_number, -1,
			      SQLITE_STATIC) != SQLITE_OK) {
		fprintf(stderr, "Failed to bind called_number\n");
		sqlite3_close(db);
		return EXIT_FAILURE;
	}

	if (sqlite3_bind_text(insert_bad_actor_stmt, 6,
			      bad_actor_event->transport_type, -1,
			      SQLITE_STATIC) != SQLITE_OK) {
		fprintf(stderr, "Failed to bind transport_type\n");
		sqlite3_close(db);
		return EXIT_FAILURE;
	}

	if (sqlite3_bind_text(insert_bad_actor_stmt, 7, bad_actor_event->method,
			      -1, SQLITE_STATIC) != SQLITE_OK) {
		fprintf(stderr, "Failed to bind method\n");
		sqlite3_close(db);
		return EXIT_FAILURE;
	}

	if (sqlite3_bind_text(insert_bad_actor_stmt, 8,
			      bad_actor_event->user_agent, -1,
			      SQLITE_STATIC) != SQLITE_OK) {
		fprintf(stderr, "Failed to bind user_agent\n");
		sqlite3_close(db);
		return EXIT_FAILURE;
	}

	if (sqlite3_bind_text(insert_bad_actor_stmt, 9,
			      bad_actor_event->sip_message, -1,
			      SQLITE_STATIC) != SQLITE_OK) {
		fprintf(stderr, "Failed to bind sip_message\n");
		sqlite3_close(db);
		return EXIT_FAILURE;
	}

	if (sqlite3_bind_text(insert_bad_actor_stmt, 10,
			      bad_actor_event->created_by_node_id, -1,
			      SQLITE_STATIC) != SQLITE_OK) {
		fprintf(stderr, "Failed to bind created_by_node_id\n");
		sqlite3_close(db);
		return EXIT_FAILURE;
	}

	if (sqlite3_step(insert_bad_actor_stmt) != SQLITE_DONE) {
		fprintf(stderr, "Error inserting bad actor: %s\n",
			sqlite3_errmsg(db));
		sqlite3_finalize(insert_bad_actor_stmt);
		sqlite3_close(db);
		return EXIT_FAILURE;
	}

	if (sqlite3_finalize(insert_bad_actor_stmt) != SQLITE_OK) {
		fprintf(stderr, "Error finalizing statement: %s\n",
			sqlite3_errmsg(db));
		sqlite3_close(db);
		return EXIT_FAILURE;
	}

	if (sqlite3_close(db) != SQLITE_OK) {
		fprintf(stderr, "Failed to close database\n");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

int db_select_bad_actor_by_ip(const char *bad_actor_ip_address,
			      sentrypeer_config const *config)
{
	return EXIT_SUCCESS;
}

int db_select_bad_actor_by_uuid(const char *bad_actor_event_uuid,
				sentrypeer_config const *config)
{
	return EXIT_SUCCESS;
}

int db_select_bad_actors(bad_actor **bad_actors, int64_t *row_count,
			     sentrypeer_config const *config)
{
	sqlite3 *db;
	assert(config->db_file);
	bad_actor *bad_actors_array = *bad_actors;

	if (sqlite3_open(config->db_file, &db) != SQLITE_OK) {
		fprintf(stderr, "Failed to open database: %s\n",
			sqlite3_errmsg(db));
		sqlite3_close(db);
		return EXIT_FAILURE;
	}

	sqlite3_stmt *get_row_count_stmt;
	char get_row_count[] = "SELECT count(*) from honey;";
	if (sqlite3_prepare_v2(db, get_row_count, -1, &get_row_count_stmt,
			       NULL) != SQLITE_OK) {
		fprintf(stderr, "Failed to prepare statement: %s\n",
			sqlite3_errmsg(db));
		sqlite3_close(db);
		return EXIT_FAILURE;
	}
	if (sqlite3_step(get_row_count_stmt) != SQLITE_ROW) {
		fprintf(stderr, "Error stepping statement: %s\n",
			sqlite3_errmsg(db));
		sqlite3_close(db);
		return EXIT_FAILURE;
	}

	*row_count = sqlite3_column_int64(get_row_count_stmt, 0);
	assert(row_count);
	if (config->debug_mode || config->verbose_mode) {
#ifdef __linux__
		fprintf(stderr, "Row count in honey table is: %ld\n",
			*row_count);
#endif
#ifdef __APPLE__
		fprintf(stderr, "Row count in honey table is: %lld\n",
			*row_count);
#endif
	}

	if (sqlite3_finalize(get_row_count_stmt) != SQLITE_OK) {
		fprintf(stderr, "Error finalizing statement: %s\n",
			sqlite3_errmsg(db));
		sqlite3_close(db);
		return EXIT_FAILURE;
	}

	sqlite3_stmt *select_bad_actors_stmt;
	char select_bad_actors[] = "SELECT * FROM honey;";
	if (sqlite3_prepare_v2(db, select_bad_actors, -1,
			       &select_bad_actors_stmt, NULL) != SQLITE_OK) {
		fprintf(stderr, "Failed to prepare statement: %s\n",
			sqlite3_errmsg(db));
		sqlite3_close(db);
		return EXIT_FAILURE;
	}

	bad_actors_array = calloc(*row_count, sizeof(bad_actor));
	assert(bad_actors_array);

	int64_t row_num = 0;
	while (row_num < *row_count) {
		if (sqlite3_step(select_bad_actors_stmt) != SQLITE_ROW) {
			fprintf(stderr, "Error stepping statement: %s\n",
				sqlite3_errmsg(db));
			bad_actors_destroy(&bad_actors_array, row_count);
			sqlite3_close(db);
			return EXIT_FAILURE;
		}

		if (config->debug_mode || config->verbose_mode) {
			fprintf(stderr, "Column name is '%s', with value: %s\n",
				sqlite3_column_name(select_bad_actors_stmt, 4),
				sqlite3_column_text(select_bad_actors_stmt, 4));
		}
		const unsigned char *source_ip =
			sqlite3_column_text(select_bad_actors_stmt,
					    4); // source_ip
		bad_actors_array[row_num].source_ip = strdup((const char *)source_ip);

		row_num++;
	}
	assert(bad_actors_array);

	if (sqlite3_finalize(select_bad_actors_stmt) != SQLITE_OK) {
		fprintf(stderr, "Error finalizing statement: %s\n",
			sqlite3_errmsg(db));
		bad_actors_destroy(&bad_actors_array, row_count);
		sqlite3_close(db);
		return EXIT_FAILURE;
	}

	if (sqlite3_close(db) != SQLITE_OK) {
		bad_actors_destroy(&bad_actors_array, row_count);
		fprintf(stderr, "Failed to close database\n");
		return EXIT_FAILURE;
	}
	*bad_actors = bad_actors_array;
	return EXIT_SUCCESS;
}
