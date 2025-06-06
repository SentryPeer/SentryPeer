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

#define _GNU_SOURCE
#include <stdio.h>

#include "database.h"
#include <stdlib.h>
#include <assert.h>
#include <stdint.h>
#include <string.h>
#include <inttypes.h>

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

int db_insert_bad_actor(bad_actor const *bad_actor_event,
			sentrypeer_config const *config)
{
	sqlite3 *db;
	sqlite3_stmt *insert_bad_actor_stmt = 0;

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

	if (sqlite3_exec(db, create_source_ip_index, NULL, NULL, NULL) !=
	    SQLITE_OK) {
		fprintf(stderr, "Failed to create source_ip_index\n");
		sqlite3_close(db);
		return EXIT_FAILURE;
	}

	if (sqlite3_exec(db, create_called_number_index, NULL, NULL, NULL) !=
	    SQLITE_OK) {
		fprintf(stderr, "Failed to create called_number_index\n");
		sqlite3_close(db);
		return EXIT_FAILURE;
	}

	if (sqlite3_exec(db, create_event_uuid_index, NULL, NULL, NULL) !=
	    SQLITE_OK) {
		fprintf(stderr, "Failed to create event_uuid_index\n");
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

bool db_bad_actor_exists(const char *bad_actor_event_uuid,
			 sentrypeer_config const *config)
{
	if (bad_actor_event_uuid == NULL) {
		if (config->debug_mode || config->verbose_mode) {
			fprintf(stderr, "Event UUID is NULL.\n");
		}
		return false;
	}

	if (is_valid_uuid(bad_actor_event_uuid)) {
		sqlite3 *db;
		assert(config->db_file);

		if (sqlite3_open(config->db_file, &db) != SQLITE_OK) {
			fprintf(stderr, "Failed to open database: %s\n",
				sqlite3_errmsg(db));
			sqlite3_close(db);
			return false;
		}

		sqlite3_stmt *find_bad_actor_stmt = 0;
		if (sqlite3_prepare_v2(db, BAD_ACTOR_EXISTS, -1,
				       &find_bad_actor_stmt,
				       NULL) != SQLITE_OK) {
			fprintf(stderr, "Failed to prepare statement: %s\n",
				sqlite3_errmsg(db));
			sqlite3_close(db);
			return false;
		}

		if (sqlite3_bind_text(find_bad_actor_stmt, 1,
				      bad_actor_event_uuid, -1,
				      SQLITE_STATIC) != SQLITE_OK) {
			fprintf(stderr, "Failed to bind event_uuid: %s\n",
				sqlite3_errmsg(db));
			sqlite3_close(db);
			return false;
		}

		if (sqlite3_step(find_bad_actor_stmt) != SQLITE_ROW) {
			sqlite3_finalize(find_bad_actor_stmt);
			sqlite3_close(db);
			return false;
		}

		// 1 found (true), 0 not found (false) - SELECT EXISTS returns int 1 or 0
		int32_t found = sqlite3_column_int(find_bad_actor_stmt, 0);

		if (sqlite3_finalize(find_bad_actor_stmt) != SQLITE_OK) {
			fprintf(stderr, "Error finalizing statement: %s\n",
				sqlite3_errmsg(db));
			sqlite3_close(db);
			return false;
		}

		if (sqlite3_close(db) != SQLITE_OK) {
			fprintf(stderr, "Failed to close database\n");
			return false;
		}

		// Found
		if (found) {
			if (config->debug_mode || config->verbose_mode) {
				fprintf(stderr,
					"Found event_uuid in honey table: %s\n",
					bad_actor_event_uuid);
			}
			return true;
		} else {
			// Not Found
			if (config->debug_mode || config->verbose_mode) {
				fprintf(stderr,
					"Event UUID does not exist in honey table: %s\n",
					bad_actor_event_uuid);
			}
			return false;
		}
	} else {
		if (config->debug_mode || config->verbose_mode) {
			fprintf(stderr, "Event UUID is not a valid UUID: %s\n",
				bad_actor_event_uuid);
		}
		return false;
	}
}

int db_select_bad_actor_by_ip(const char *bad_actor_ip_address,
			      bad_actor **bad_actor_to_find,
			      sentrypeer_config const *config)
{
	sqlite3 *db;
	assert(config->db_file);

	if (sqlite3_open(config->db_file, &db) != SQLITE_OK) {
		fprintf(stderr, "Failed to open database: %s\n",
			sqlite3_errmsg(db));
		sqlite3_close(db);
		return EXIT_FAILURE;
	}

	sqlite3_stmt *find_bad_actor_stmt = 0;
	if (sqlite3_prepare_v2(db, GET_BAD_ACTOR_BY_IP, -1,
			       &find_bad_actor_stmt, NULL) != SQLITE_OK) {
		fprintf(stderr, "Failed to prepare statement: %s\n",
			sqlite3_errmsg(db));
		sqlite3_close(db);
		return EXIT_FAILURE;
	}
	bad_actor *bad_actor_found =
		bad_actor_new(0, 0, 0, 0, 0, 0, 0, 0, config->node_id);
	assert(bad_actor_found);

	if (sqlite3_bind_text(find_bad_actor_stmt, 1, bad_actor_ip_address, -1,
			      SQLITE_STATIC) != SQLITE_OK) {
		fprintf(stderr, "Failed to bind IP address: %s\n",
			sqlite3_errmsg(db));
		bad_actor_destroy(&bad_actor_found);
		sqlite3_close(db);
		return EXIT_FAILURE;
	}

	// Nothing found. No need to free bad_actor_found->source_ip
	if (sqlite3_step(find_bad_actor_stmt) != SQLITE_ROW) {
		sqlite3_finalize(find_bad_actor_stmt);
		bad_actor_destroy(&bad_actor_found);
		sqlite3_close(db);
		return EXIT_FAILURE;
	}

	const unsigned char *source_ip =
		sqlite3_column_text(find_bad_actor_stmt,
				    0); // source_ip needs to be freed
	bad_actor_found->source_ip =
		util_duplicate_string((const char *)source_ip);
	assert(bad_actor_found->source_ip);

	if (config->debug_mode || config->verbose_mode) {
		fprintf(stderr, "Found source_ip in honey table: %s\n",
			bad_actor_found->source_ip);
	}

	if (sqlite3_finalize(find_bad_actor_stmt) != SQLITE_OK) {
		fprintf(stderr, "Error finalizing statement: %s\n",
			sqlite3_errmsg(db));
		bad_actor_destroy(&bad_actor_found);
		sqlite3_close(db);
		return EXIT_FAILURE;
	}

	if (sqlite3_close(db) != SQLITE_OK) {
		fprintf(stderr, "Failed to close database\n");
		bad_actor_destroy(&bad_actor_found);
		return EXIT_FAILURE;
	}

	*bad_actor_to_find = bad_actor_found;
	return EXIT_SUCCESS;
}

int db_select_bad_actors(bad_actor ***bad_actors, int64_t *row_count,
			 sentrypeer_config const *config)
{
	sqlite3 *db;
	assert(config->db_file);

	if (sqlite3_open(config->db_file, &db) != SQLITE_OK) {
		fprintf(stderr, "Failed to open database: %s\n",
			sqlite3_errmsg(db));
		sqlite3_close(db);
		return EXIT_FAILURE;
	}

	sqlite3_stmt *get_row_count_stmt = 0;
	if (sqlite3_prepare_v2(db, GET_ROWS_DISTINCT_SOURCE_IP_COUNT, -1,
			       &get_row_count_stmt, NULL) != SQLITE_OK) {
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
		fprintf(stderr,
			"Distinct source_ip row count in honey table is: %" PRId64
			"\n",
			*row_count);
	}

	if (sqlite3_finalize(get_row_count_stmt) != SQLITE_OK) {
		fprintf(stderr, "Error finalizing statement: %s\n",
			sqlite3_errmsg(db));
		sqlite3_close(db);
		return EXIT_FAILURE;
	}

	sqlite3_stmt *select_bad_actors_stmt = 0;
	char select_bad_actors[] =
		GET_ROWS_DISTINCT_SOURCE_IP_WITH_COUNT_AND_DATE;
	if (sqlite3_prepare_v2(db, select_bad_actors, -1,
			       &select_bad_actors_stmt, NULL) != SQLITE_OK) {
		fprintf(stderr, "Failed to prepare statement: %s\n",
			sqlite3_errmsg(db));
		sqlite3_close(db);
		return EXIT_FAILURE;
	}

	bad_actor **bad_actors_array =
		calloc(*row_count, sizeof(*bad_actors_array));
	assert(bad_actors_array);

	int64_t row_num = 0;
	while (row_num < *row_count) {
		if (sqlite3_step(select_bad_actors_stmt) != SQLITE_ROW) {
			fprintf(stderr, "Error stepping statement: %s\n",
				sqlite3_errmsg(db));
			free(bad_actors_array); // Nothing in there yet
			sqlite3_close(db);
			return EXIT_FAILURE;
		}

		if (config->debug_mode || config->verbose_mode) {
			fprintf(stderr, "Column name is '%s', with value: %s\n",
				sqlite3_column_name(select_bad_actors_stmt, 0),
				sqlite3_column_text(select_bad_actors_stmt, 0));
		}
		const unsigned char *source_ip =
			sqlite3_column_text(select_bad_actors_stmt,
					    0); // source_ip

		const unsigned char *seen_last =
			sqlite3_column_text(select_bad_actors_stmt,
					    1); // seen_last

		const unsigned char *seen_count =
			sqlite3_column_text(select_bad_actors_stmt,
					    2); // seen_count

		bad_actors_array[row_num] = bad_actor_new(
			0, util_duplicate_string((const char *)source_ip), 0, 0,
			0, 0, 0, 0, config->node_id);

		bad_actors_array[row_num]->seen_last =
			util_duplicate_string((const char *)seen_last);
		bad_actors_array[row_num]->seen_count =
			util_duplicate_string((const char *)seen_count);

		row_num++;
	}
	assert(bad_actors_array);

	if (sqlite3_finalize(select_bad_actors_stmt) != SQLITE_OK) {
		fprintf(stderr, "Error finalizing statement: %s\n",
			sqlite3_errmsg(db));
		bad_actors_destroy(bad_actors_array, row_count);
		free(bad_actors_array);
		sqlite3_close(db);
		return EXIT_FAILURE;
	}

	if (sqlite3_close(db) != SQLITE_OK) {
		bad_actors_destroy(bad_actors_array, row_count);
		free(bad_actors_array);
		fprintf(stderr, "Failed to close database\n");
		return EXIT_FAILURE;
	}
	*bad_actors = bad_actors_array;
	return EXIT_SUCCESS;
}

int db_select_called_numbers(bad_actor ***phone_numbers, int64_t *row_count,
			     sentrypeer_config const *config)
{
	sqlite3 *db;
	assert(config->db_file);

	if (sqlite3_open(config->db_file, &db) != SQLITE_OK) {
		fprintf(stderr, "Failed to open database: %s\n",
			sqlite3_errmsg(db));
		sqlite3_close(db);
		return EXIT_FAILURE;
	}

	sqlite3_stmt *get_row_count_stmt = 0;
	if (sqlite3_prepare_v2(db, GET_ROWS_DISTINCT_PHONE_NUMBER_COUNT, -1,
			       &get_row_count_stmt, NULL) != SQLITE_OK) {
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
		fprintf(stderr,
			"Distinct called_number row count in honey table is: %" PRId64
			"\n",
			*row_count);
	}

	if (sqlite3_finalize(get_row_count_stmt) != SQLITE_OK) {
		fprintf(stderr, "Error finalizing statement: %s\n",
			sqlite3_errmsg(db));
		sqlite3_close(db);
		return EXIT_FAILURE;
	}

	sqlite3_stmt *select_phone_numbers_stmt = 0;
	char select_called_numbers[] =
		GET_ROWS_DISTINCT_PHONE_NUMBER_WITH_COUNT_AND_DATE;
	if (sqlite3_prepare_v2(db, select_called_numbers, -1,
			       &select_phone_numbers_stmt, NULL) != SQLITE_OK) {
		fprintf(stderr, "Failed to prepare statement: %s\n",
			sqlite3_errmsg(db));
		sqlite3_close(db);
		return EXIT_FAILURE;
	}

	bad_actor **phone_numbers_array =
		calloc(*row_count, sizeof(*phone_numbers_array));
	assert(phone_numbers_array);

	int64_t row_num = 0;
	while (row_num < *row_count) {
		if (sqlite3_step(select_phone_numbers_stmt) != SQLITE_ROW) {
			fprintf(stderr, "Error stepping statement: %s\n",
				sqlite3_errmsg(db));
			free(phone_numbers_array); // Nothing in there yet
			sqlite3_close(db);
			return EXIT_FAILURE;
		}

		if (config->debug_mode || config->verbose_mode) {
			fprintf(stderr, "Column name is '%s', with value: %s\n",
				sqlite3_column_name(select_phone_numbers_stmt,
						    0),
				sqlite3_column_text(select_phone_numbers_stmt,
						    0));
		}
		const unsigned char *called_number =
			sqlite3_column_text(select_phone_numbers_stmt,
					    0); // called_number

		const unsigned char *seen_last =
			sqlite3_column_text(select_phone_numbers_stmt,
					    1); // seen_last

		const unsigned char *seen_count =
			sqlite3_column_text(select_phone_numbers_stmt,
					    2); // seen_count

		phone_numbers_array[row_num] = bad_actor_new(
			0, 0, 0,
			util_duplicate_string((const char *)called_number), 0,
			0, 0, 0, config->node_id);

		phone_numbers_array[row_num]->seen_last =
			util_duplicate_string((const char *)seen_last);
		phone_numbers_array[row_num]->seen_count =
			util_duplicate_string((const char *)seen_count);

		row_num++;
	}
	assert(phone_numbers_array);

	if (sqlite3_finalize(select_phone_numbers_stmt) != SQLITE_OK) {
		fprintf(stderr, "Error finalizing statement: %s\n",
			sqlite3_errmsg(db));
		bad_actors_destroy(phone_numbers_array, row_count);
		free(phone_numbers_array);
		sqlite3_close(db);
		return EXIT_FAILURE;
	}

	if (sqlite3_close(db) != SQLITE_OK) {
		bad_actors_destroy(phone_numbers_array, row_count);
		free(phone_numbers_array);
		fprintf(stderr, "Failed to close database\n");
		return EXIT_FAILURE;
	}
	*phone_numbers = phone_numbers_array;
	return EXIT_SUCCESS;
}

int db_select_phone_number(const char *phone_number,
			   bad_actor **phone_number_to_find,
			   sentrypeer_config const *config)
{
	sqlite3 *db;
	assert(config->db_file);

	if (sqlite3_open(config->db_file, &db) != SQLITE_OK) {
		fprintf(stderr, "Failed to open database: %s\n",
			sqlite3_errmsg(db));
		sqlite3_close(db);
		return EXIT_FAILURE;
	}

	sqlite3_stmt *find_phone_number_stmt = 0;
	if (sqlite3_prepare_v2(db, GET_PHONE_NUMBER, -1,
			       &find_phone_number_stmt, NULL) != SQLITE_OK) {
		fprintf(stderr, "Failed to prepare statement: %s\n",
			sqlite3_errmsg(db));
		sqlite3_close(db);
		return EXIT_FAILURE;
	}
	bad_actor *phone_number_found =
		bad_actor_new(0, 0, 0, 0, 0, 0, 0, 0, config->node_id);
	assert(phone_number_found);

	if (sqlite3_bind_text(find_phone_number_stmt, 1, phone_number, -1,
			      SQLITE_STATIC) != SQLITE_OK) {
		fprintf(stderr, "Failed to bind called_number: %s\n",
			sqlite3_errmsg(db));
		bad_actor_destroy(&phone_number_found);
		sqlite3_close(db);
		return EXIT_FAILURE;
	}

	// Nothing found. No need to free bad_actor_found->source_ip
	if (sqlite3_step(find_phone_number_stmt) != SQLITE_ROW) {
		sqlite3_finalize(find_phone_number_stmt);
		bad_actor_destroy(&phone_number_found);
		sqlite3_close(db);
		return EXIT_FAILURE;
	}

	const unsigned char *called_number =
		sqlite3_column_text(find_phone_number_stmt,
				    0); // called_number needs to be freed
	phone_number_found->called_number =
		util_duplicate_string((const char *)called_number);
	assert(phone_number_found->called_number);

	if (config->debug_mode || config->verbose_mode) {
		fprintf(stderr, "Found called_number in honey table: %s\n",
			phone_number_found->called_number);
	}

	if (sqlite3_finalize(find_phone_number_stmt) != SQLITE_OK) {
		fprintf(stderr, "Error finalizing statement: %s\n",
			sqlite3_errmsg(db));
		bad_actor_destroy(&phone_number_found);
		sqlite3_close(db);
		return EXIT_FAILURE;
	}

	if (sqlite3_close(db) != SQLITE_OK) {
		fprintf(stderr, "Failed to close database\n");
		bad_actor_destroy(&phone_number_found);
		return EXIT_FAILURE;
	}

	*phone_number_to_find = phone_number_found;
	return EXIT_SUCCESS;
}
