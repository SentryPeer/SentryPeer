//
// Created by ghenry on 17/12/2021.
//

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

#include <microhttpd.h>
#include <jansson.h>
#include "config.h"

#include "http_common.h"
#include "bad_actor.h"
#include "database.h"

int ip_addresses_route(struct MHD_Connection *connection,
		       sentrypeer_config *config)
{
	const char *reply = 0;
	bad_actor *bad_actors = 0;
	int64_t row_count = 0;

	if (db_select_bad_actors(&bad_actors, &row_count, config) !=
	    EXIT_SUCCESS) {
		fprintf(stderr, "Failed to select bad actors from database\n");
		return MHD_NO;
	}

	if ((bad_actors != 0) && (row_count > 0)) {
		int64_t row_num = 0;
		json_t *json_arr = json_array();

		while (row_num < row_count) {
			fprintf(stderr, "source_ip: %s\n",
				bad_actors[row_num].source_ip);
			json_array_append(
				json_arr,
				json_pack("{s:s}", "ip_address",

					  bad_actors[row_num].source_ip));
			row_num++;
		}
		json_t *json_final_obj =
			json_pack("{s:i,s:o}", "ip_addresses_total", row_count,
				  "ip_addresses", json_arr);
		reply = json_dumps(json_final_obj, JSON_INDENT(2));

		// Free the json objects
		json_decref(json_arr);
		json_decref(json_final_obj);
		bad_actors_destroy(&bad_actors, &row_count);

		return finalise_response(connection, reply, CONTENT_TYPE_JSON,
					 MHD_HTTP_OK);
	} else {
		json_t *json_no_data =
			json_pack("{s:s}", "message", "No bad actors found");
		reply = json_dumps(json_no_data, JSON_INDENT(2));

		// Free the json object
		json_decref(json_no_data);
		bad_actors_destroy(&bad_actors, &row_count);

		return finalise_response(connection, reply, CONTENT_TYPE_JSON,
					 MHD_HTTP_NOT_FOUND);
	}
}
