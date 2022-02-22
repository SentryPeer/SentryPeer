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

int json_log_bad_actor(const sentrypeer_config *config,
		       const bad_actor *bad_actor_to_log)
{
	FILE *logfile = fopen(config->json_log_file, "a");
	if (logfile == NULL) {
		fprintf(stderr, "Could not open JSON log file: %s\n",
			config->json_log_file);
		return EXIT_FAILURE;
	}

	json_error_t error;
	json_t *json_bad_actor = json_pack(
		"{s:s,s:s,s:s,s:s,s:s,s:s,s:s,s:s,s:s,s:s,s:s,s:s,s:s}",
		"app_name", PACKAGE_NAME, "app_version", PACKAGE_VERSION,
		"event_timestamp",
		bad_actor_to_log->event_timestamp ?
			bad_actor_to_log->event_timestamp :
			      "",
		"event_uuid",
		bad_actor_to_log->event_uuid ? bad_actor_to_log->event_uuid :
						     "",
		"created_by_node_id",
		bad_actor_to_log->created_by_node_id ?
			bad_actor_to_log->created_by_node_id :
			      "",
		"collected_method",
		bad_actor_to_log->collected_method ?
			bad_actor_to_log->collected_method :
			      "",
		"transport_type",
		bad_actor_to_log->transport_type ?
			bad_actor_to_log->transport_type :
			      "",
		"source_ip",
		bad_actor_to_log->source_ip ? bad_actor_to_log->source_ip : "",
		"destination_ip",
		bad_actor_to_log->destination_ip ?
			bad_actor_to_log->destination_ip :
			      "",
		"called_number",
		bad_actor_to_log->called_number ?
			bad_actor_to_log->called_number :
			      "",
		"sip_method",
		bad_actor_to_log->method ? bad_actor_to_log->method : "",
		"sip_user_agent",
		bad_actor_to_log->user_agent ? bad_actor_to_log->user_agent :
						     "",
		"sip_message",
		bad_actor_to_log->sip_message ? bad_actor_to_log->sip_message :
						      "",
		&error);
	if (!json_bad_actor) {
		fprintf(stderr, "Error creating json log object: %s\n",
			error.text);
		fclose(logfile);
		return EXIT_FAILURE;
	}

	char *json_string = json_dumps(json_bad_actor, JSON_COMPACT);
	fprintf(logfile, "%s\n", json_string);

	json_decref(json_bad_actor);
	free(json_string);

	if (fclose(logfile) != EXIT_SUCCESS) {
		fprintf(stderr, "Could not close JSON log file: %s\n",
			config->json_log_file);
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
