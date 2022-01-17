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

int ip_address_route(char **ip_address, struct MHD_Connection *connection,
		     sentrypeer_config const *config)
{
	bad_actor *bad_actor_found = 0;
	char *ip_address_str = *ip_address;

	if (db_select_bad_actor_by_ip(ip_address_str, &bad_actor_found,
				      config) != EXIT_SUCCESS) {
		// Free the objects
		free(*ip_address);
		*ip_address = 0;
		bad_actor_destroy(&bad_actor_found);
		return finalise_response(connection, NOT_FOUND_BAD_ACTOR_JSON,
					 CONTENT_TYPE_JSON, MHD_HTTP_NOT_FOUND,
					 false);
	} else { // Found!!!!
		if (config->verbose_mode || config->debug_mode) {
			fprintf(stderr, "bad_actor IP found: %s\n",
				bad_actor_found->source_ip);
		}

		json_t *json_final_obj = json_pack("{s:s}", "bad_actor_found",
						   bad_actor_found->source_ip);

		const char *reply = json_dumps(json_final_obj, JSON_INDENT(2));

		// Free the objects
		free(*ip_address);
		*ip_address = 0;
		json_decref(json_final_obj);
		bad_actor_destroy(&bad_actor_found);

		return finalise_response(connection, reply, CONTENT_TYPE_JSON,
					 MHD_HTTP_OK, true);
	}
}
