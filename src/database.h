/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only  */
/* Copyright (c) 2021 - 2022 Gavin Henry <ghenry@sentrypeer.org> */
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

#ifndef SENTRYPEER_DATABASE_H
#define SENTRYPEER_DATABASE_H 1

#include <sqlite3.h>

#include "bad_actor.h"
#include "conf.h"

#define DEFAULT_DB_FILE_NAME "sentrypeer.db"

int db_insert_bad_actor(bad_actor const *bad_actor_event,
			sentrypeer_config const *config);

#define GET_BAD_ACTOR_BY_IP                                                    \
	"SELECT DISTINCT(source_ip) FROM honey WHERE source_ip = ?;"
int db_select_bad_actor_by_ip(const char *bad_actor_ip_address,
			      bad_actor **bad_actor,
			      sentrypeer_config const *config);

#define GET_BAD_ACTOR_BY_IP_WITH_DETAILS                                       \
	"SELECT DISTINCT(source_ip), max(event_timestamp) as seen_last, count(source_ip) as seen_total FROM honey WHERE source_ip = ? order by event_timestamp DESC;"

#define GET_ROWS_DISTINCT_SOURCE_IP_COUNT                                      \
	"SELECT COUNT(DISTINCT source_ip) from honey;"
#define GET_ROWS_DISTINCT_SOURCE_IP_WITH_COUNT_AND_DATE                        \
	"SELECT source_ip, max(event_timestamp) as seen_last, count(source_ip) as seen_total FROM honey GROUP BY source_ip order by event_timestamp DESC;"
int db_select_bad_actors(bad_actor ***bad_actors, int64_t *row_count,
			 sentrypeer_config const *config);

#define GET_PHONE_NUMBER                                                       \
	"SELECT DISTINCT(called_number) FROM honey WHERE called_number = ?;"
int db_select_phone_number(const char *phone_number,
			   bad_actor **phone_number_to_find,
			   sentrypeer_config const *config);

#define GET_ROWS_DISTINCT_PHONE_NUMBER_COUNT                                   \
	"SELECT COUNT(DISTINCT called_number) from honey;"
#define GET_ROWS_DISTINCT_PHONE_NUMBER_WITH_COUNT_AND_DATE                     \
	"SELECT called_number, max(event_timestamp) as seen_last, count(called_number) as seen_total FROM honey WHERE called_number is not NULL GROUP BY called_number order by event_timestamp DESC;"
int db_select_called_numbers(bad_actor ***phone_numbers, int64_t *row_count,
			     sentrypeer_config const *config);

#endif //SENTRYPEER_DATABASE_H
