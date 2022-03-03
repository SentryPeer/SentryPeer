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

#include <microhttpd.h>
#include <jansson.h>
#include "config.h"

#include "http_common.h"
#include "bad_actor.h"
#include "database.h"

int called_numbers_route(struct MHD_Connection *connection,
			 sentrypeer_config const *config)
{
	const char *reply = NULL;
	bad_actor **phone_numbers = 0;
	int64_t row_count = 0;

	if (db_select_called_numbers(&phone_numbers, &row_count, config) !=
	    EXIT_SUCCESS) {
		fprintf(stderr,
			"Failed to select called numbers from database\n");
		return finalise_response(connection,
					 NOT_FOUND_PHONE_NUMBERS_JSON,
					 CONTENT_TYPE_JSON, MHD_HTTP_NOT_FOUND,
					 false);
	}

	if ((phone_numbers != 0) && (row_count > 0)) {
		int64_t row_num = 0;
		json_t *json_arr = json_array();

		while (row_num < row_count) {
			if (config->verbose_mode || config->debug_mode) {
				fprintf(stderr, "called_number: %s\n",
					phone_numbers[row_num]->called_number);
			}

			json_error_t error;
			if (json_array_append_new(
				    json_arr,
				    json_pack("{s:s,s:s,s:s}", "called_number",

					      phone_numbers[row_num]
						      ->called_number,
					      "seen_last",
					      phone_numbers[row_num]->seen_last,
					      "seen_count",
					      phone_numbers[row_num]->seen_count,
					      &error)) != EXIT_SUCCESS) {
				fprintf(stderr,
					"Failed to append called_number to json array\n");
				fprintf(stderr, "Error: %s\n", error.text);

				// Free the json objects
				json_decref(json_arr);
				bad_actors_destroy(phone_numbers, &row_count);
				free(phone_numbers);

				return finalise_response(
					connection,
					NOT_FOUND_PHONE_NUMBERS_JSON,
					CONTENT_TYPE_JSON, MHD_HTTP_NOT_FOUND,
					false);
			}

			row_num++;
		}
		json_t *json_final_obj =
			json_pack("{s:i,s:o}", "called_numbers_total",
				  row_count, "called_numbers", json_arr);
		reply = json_dumps(json_final_obj, JSON_INDENT(2));

		// Free the json objects
		json_decref(json_final_obj);
		bad_actors_destroy(phone_numbers, &row_count);
		free(phone_numbers);

		// TODO: Add compression if client supports it and is large enough
		// https://fossies.org/linux/libmicrohttpd/src/examples/http_compression.c
		return finalise_response(connection, reply, CONTENT_TYPE_JSON,
					 MHD_HTTP_OK, true);
	} else {
		bad_actors_destroy(phone_numbers, &row_count);
		free(phone_numbers);
		return finalise_response(connection,
					 NOT_FOUND_PHONE_NUMBERS_JSON,
					 CONTENT_TYPE_JSON, MHD_HTTP_NOT_FOUND,
					 false);
	}
}
