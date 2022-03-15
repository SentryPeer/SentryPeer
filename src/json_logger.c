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

#include <stdio.h>
#include <jansson.h>

#include "json_logger.h"
#include "config.h"
#include <assert.h>

char *bad_actor_to_json(const sentrypeer_config *config,
			const bad_actor *bad_actor_to_convert)
{
	json_error_t error;
	json_t *json_bad_actor = json_pack(
		"{s:s,s:s,s:s,s:s,s:s,s:s,s:s,s:s,s:s,s:s,s:s,s:s,s:s}",
		"app_name", PACKAGE_NAME, "app_version", PACKAGE_VERSION,
		"event_timestamp",
		bad_actor_to_convert->event_timestamp ?
			bad_actor_to_convert->event_timestamp :
			      "",
		"event_uuid",
		bad_actor_to_convert->event_uuid ?
			bad_actor_to_convert->event_uuid :
			      "",
		"created_by_node_id",
		bad_actor_to_convert->created_by_node_id ?
			bad_actor_to_convert->created_by_node_id :
			      "",
		"collected_method",
		bad_actor_to_convert->collected_method ?
			bad_actor_to_convert->collected_method :
			      "",
		"transport_type",
		bad_actor_to_convert->transport_type ?
			bad_actor_to_convert->transport_type :
			      "",
		"source_ip",
		bad_actor_to_convert->source_ip ?
			bad_actor_to_convert->source_ip :
			      "",
		"destination_ip",
		bad_actor_to_convert->destination_ip ?
			bad_actor_to_convert->destination_ip :
			      "",
		"called_number",
		bad_actor_to_convert->called_number ?
			bad_actor_to_convert->called_number :
			      "",
		"sip_method",
		bad_actor_to_convert->method ? bad_actor_to_convert->method :
						     "",
		"sip_user_agent",
		bad_actor_to_convert->user_agent ?
			bad_actor_to_convert->user_agent :
			      "",
		"sip_message",
		bad_actor_to_convert->sip_message ?
			bad_actor_to_convert->sip_message :
			      "",
		&error);
	if (!json_bad_actor) {
		fprintf(stderr, "Error creating json log object: %s\n",
			error.text);
		return NULL;
	}

	char *json_string = json_dumps(json_bad_actor, JSON_COMPACT);
	json_decref(json_bad_actor);

	if (config->debug_mode || config->verbose_mode) {
		fprintf(stderr, "Bad actor in JSON format: %s\n", json_string);
	}

	return json_string; // Caller must free
}

int json_log_bad_actor(const sentrypeer_config *config,
		       const bad_actor *bad_actor_to_log)
{
	FILE *logfile = fopen(config->json_log_file, "a");
	if (logfile == NULL) {
		fprintf(stderr, "Could not open JSON log file: %s\n",
			config->json_log_file);
		return EXIT_FAILURE;
	}

	char *json_string =
		bad_actor_to_json(config,
				  bad_actor_to_log); // Caller must free
	assert(json_string);

	fprintf(logfile, "%s\n", json_string);
	free(json_string);

	if (fclose(logfile) != EXIT_SUCCESS) {
		fprintf(stderr, "Could not close JSON log file: %s\n",
			config->json_log_file);
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
