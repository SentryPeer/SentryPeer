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
#include <string.h>

#include "json_logger.h"
#include "config.h"
#include <assert.h>
#include <curl/curl.h>

#define BAD_ACTOR_JSON_FMT                                                     \
	"{s:s,s:s,s:s,s:s,s:s,s:s,s:s,s:s,s:s,s:s,s:s,s:s,s:s}"
#define SENTRYPEER_USERAGENT "SentryPeer/1.0"

char *bad_actor_to_json(const sentrypeer_config *config,
			const bad_actor *bad_actor_to_convert)
{
	json_error_t error;
	json_t *json_bad_actor =
		json_pack(BAD_ACTOR_JSON_FMT, "app_name", PACKAGE_NAME,
			  "app_version", PACKAGE_VERSION, "event_timestamp",
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
			  bad_actor_to_convert->method ?
				  bad_actor_to_convert->method :
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

bad_actor *json_to_bad_actor(const sentrypeer_config *config,
			     const char *json_to_convert)
{
	if (config->debug_mode || config->verbose_mode) {
		fprintf(stderr, "JSON to convert to a bad_actor: %s\n",
			json_to_convert);
	}
	assert(json_to_convert);

	json_error_t error;
	// Try to load the JSON string into a JSON object
	json_t *json_bad_actor =
		json_loadb(json_to_convert, strlen(json_to_convert), 0, &error);
	if (!json_bad_actor) {
		fprintf(stderr, "Error converting string to JSON: %s\n",
			error.text);
		json_decref(json_bad_actor);
		return NULL;
	}

	// Check bad_actor keys are all present
	const char *bad_actor_keys[] = {
		"app_name",	  "app_version",	"event_timestamp",
		"event_uuid",	  "created_by_node_id", "collected_method",
		"transport_type", "source_ip",		"destination_ip",
		"called_number",  "sip_method",		"sip_user_agent",
		"sip_message"
	};
	if (json_unpack_ex(
		    json_bad_actor, &error, JSON_VALIDATE_ONLY,
		    BAD_ACTOR_JSON_FMT, bad_actor_keys[0], bad_actor_keys[1],
		    bad_actor_keys[2], bad_actor_keys[3], bad_actor_keys[4],
		    bad_actor_keys[5], bad_actor_keys[6], bad_actor_keys[7],
		    bad_actor_keys[8], bad_actor_keys[9], bad_actor_keys[10],
		    bad_actor_keys[11], bad_actor_keys[12]) != EXIT_SUCCESS) {
		fprintf(stderr, "Error validating JSON. %s\n", error.text);
		json_decref(json_bad_actor);
		return NULL;
	}

	bad_actor *bad_actor_event =
		bad_actor_new(0, 0, 0, 0, 0, 0, 0, 0, config->node_id);
	assert(bad_actor_event);

	// Let's free up event_timestamp and event_uuid as we don't need them as we're taking them
	// from the JSON off the DHT
	free(bad_actor_event->event_timestamp);
	bad_actor_event->event_timestamp = 0;

	free(bad_actor_event->event_uuid);
	bad_actor_event->event_uuid = 0;

	free(bad_actor_event->created_by_node_id);
	bad_actor_event->created_by_node_id = 0;

	bad_actor_event->event_timestamp =
		util_duplicate_string(json_string_value(
			json_object_get(json_bad_actor, bad_actor_keys[2])));
	bad_actor_event->event_uuid = util_duplicate_string(json_string_value(
		json_object_get(json_bad_actor, bad_actor_keys[3])));
	bad_actor_event->created_by_node_id =
		util_duplicate_string(json_string_value(
			json_object_get(json_bad_actor, bad_actor_keys[4])));
	bad_actor_event->collected_method =
		util_duplicate_string(json_string_value(
			json_object_get(json_bad_actor, bad_actor_keys[5])));
	bad_actor_event->transport_type =
		util_duplicate_string(json_string_value(
			json_object_get(json_bad_actor, bad_actor_keys[6])));
	bad_actor_event->source_ip = util_duplicate_string(json_string_value(
		json_object_get(json_bad_actor, bad_actor_keys[7])));
	bad_actor_event->destination_ip =
		util_duplicate_string(json_string_value(
			json_object_get(json_bad_actor, bad_actor_keys[8])));
	bad_actor_event->called_number =
		util_duplicate_string(json_string_value(
			json_object_get(json_bad_actor, bad_actor_keys[9])));
	bad_actor_event->method = util_duplicate_string(json_string_value(
		json_object_get(json_bad_actor, bad_actor_keys[10])));
	bad_actor_event->user_agent = util_duplicate_string(json_string_value(
		json_object_get(json_bad_actor, bad_actor_keys[11])));
	bad_actor_event->sip_message = util_duplicate_string(json_string_value(
		json_object_get(json_bad_actor, bad_actor_keys[12])));

	if (config->debug_mode || config->verbose_mode) {
		fprintf(stderr,
			"bad_actor from JSON on DHT is:\n"
			"event_timestamp: %s\n"
			"event_uuid: %s\n"
			"created_by_node_id: %s\n"
			"collected_method: %s\n"
			"transport_type: %s\n"
			"source_ip: %s\n"
			"destination_ip: %s\n"
			"called_number: %s\n"
			"sip_method: %s\n"
			"sip_user_agent: %s\n"
			"sip_message: %s\n",
			bad_actor_event->event_timestamp,
			bad_actor_event->event_uuid,
			bad_actor_event->created_by_node_id,
			bad_actor_event->collected_method,
			bad_actor_event->transport_type,
			bad_actor_event->source_ip,
			bad_actor_event->destination_ip,
			bad_actor_event->called_number, bad_actor_event->method,
			bad_actor_event->user_agent,
			bad_actor_event->sip_message);
	}
	json_decref(json_bad_actor);

	return bad_actor_event; // Caller must free
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
	// We don't assert here, because we want to continue even if it fails
	if (json_string == NULL) {
		fprintf(stderr, "Failed to convert bad actor to json.\n");
		free(json_string);
		fclose(logfile);
		return EXIT_FAILURE;
	}
	fprintf(logfile, "%s\n", json_string);
	free(json_string);

	if (fclose(logfile) != EXIT_SUCCESS) {
		fprintf(stderr, "Could not close JSON log file: %s\n",
			config->json_log_file);
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

int json_http_post_bad_actor(const sentrypeer_config *config,
			     const bad_actor *bad_actor_to_log)
{
	CURL *curl;
	CURLcode res;

	curl_global_init(CURL_GLOBAL_ALL);
	curl = curl_easy_init();

	if (!curl) {
		fprintf(stderr, "curl_easy_init() failed\n");
		return EXIT_FAILURE;
	}

	char *json_string =
		bad_actor_to_json(config,
				  bad_actor_to_log); // Caller must free
	if (json_string == NULL) {
		fprintf(stderr, "Failed to convert bad actor to json.\n");
		free(json_string);

		curl_easy_cleanup(curl);
		curl_global_cleanup();

		return EXIT_FAILURE;
	}

	res = curl_easy_setopt(curl, CURLOPT_URL, config->webhook_url);
	if (res != CURLE_OK) {
		fprintf(stderr, "curl_easy_setopt() failed: %s\n",
			curl_easy_strerror(res));
		curl_easy_cleanup(curl);
		curl_global_cleanup();
		return EXIT_FAILURE;
	}
	res = curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_string);
	if (res != CURLE_OK) {
		fprintf(stderr, "curl_easy_setopt() failed: %s\n",
			curl_easy_strerror(res));
		curl_easy_cleanup(curl);
		curl_global_cleanup();
		return EXIT_FAILURE;
	}

	struct curl_slist *headers = 0;
	headers = curl_slist_append(headers, "Content-Type: application/json");
	res = curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
	if (res != CURLE_OK) {
		fprintf(stderr, "curl_easy_setopt() failed: %s\n",
			curl_easy_strerror(res));
		curl_easy_cleanup(curl);
		curl_global_cleanup();
		return EXIT_FAILURE;
	}

	res = curl_easy_setopt(curl, CURLOPT_USERAGENT, SENTRYPEER_USERAGENT);
	if (res != CURLE_OK) {
		fprintf(stderr, "curl_easy_setopt() failed: %s\n",
			curl_easy_strerror(res));
		curl_easy_cleanup(curl);
		curl_global_cleanup();
		return EXIT_FAILURE;
	}

	// Enables TLSv1.2 / TLSv1.3 version only
	res = curl_easy_setopt(curl, CURLOPT_SSLVERSION,
			       CURL_SSLVERSION_TLSv1_2);
	if (res != CURLE_OK) {
		fprintf(stderr, "curl_easy_setopt() failed: %s\n",
			curl_easy_strerror(res));
		curl_easy_cleanup(curl);
		curl_global_cleanup();
		return EXIT_FAILURE;
	}

	// Complete within 2 seconds
	res = curl_easy_setopt(curl, CURLOPT_TIMEOUT, 2L);
	if (res != CURLE_OK) {
		fprintf(stderr, "curl_easy_setopt() failed: %s\n",
			curl_easy_strerror(res));
		curl_easy_cleanup(curl);
		curl_global_cleanup();
		return EXIT_FAILURE;
	}

	// Send the json :-)
	res = curl_easy_perform(curl);
	if (res != CURLE_OK) {
		fprintf(stderr, "WebHook POSTing failed: %s\n",
			curl_easy_strerror(res));
		return EXIT_FAILURE;
	}

	free(json_string);

	// Cleanup curl stuff
	curl_easy_cleanup(curl);
	curl_slist_free_all(headers);

	// We are done with libcurl, so clean it up
	curl_global_cleanup();

	return EXIT_SUCCESS;
}
