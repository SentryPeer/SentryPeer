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

#include <microhttpd.h>
#include <jansson.h>
#include "config.h"

#include "http_common.h"
#include "bad_actor.h"
#include "database.h"

int called_number_route(char **phone_number, struct MHD_Connection *connection,
			sentrypeer_config const *config)
{
	bad_actor *phone_number_found = 0;
	const char *phone_number_str = *phone_number;

	if (db_select_phone_number(phone_number_str, &phone_number_found,
				   config) != EXIT_SUCCESS) {
		// Free the objects
		free(*phone_number);
		*phone_number = 0;
		bad_actor_destroy(&phone_number_found);
		return finalise_response(connection,
					 NOT_FOUND_PHONE_NUMBER_JSON,
					 CONTENT_TYPE_JSON, MHD_HTTP_NOT_FOUND,
					 false);
	} else { // Found!!!!
		if (config->verbose_mode || config->debug_mode) {
			fprintf(stderr, "Called number found: %s\n",
				phone_number_found->called_number);
		}

		json_t *json_final_obj =
			json_pack("{s:s}", "phone_number_found",
				  phone_number_found->called_number);

		const char *reply = json_dumps(json_final_obj, JSON_INDENT(2));

		// Free the objects
		free(*phone_number);
		*phone_number = 0;
		json_decref(json_final_obj);
		bad_actor_destroy(&phone_number_found);

		return finalise_response(connection, reply, CONTENT_TYPE_JSON,
					 MHD_HTTP_OK, true);
	}
}
