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

#include <stdio.h>
#include <jansson.h>
#include <string.h>

#include "json_logger.h"
#include "config.h"
#include "http_common.h"
#include <assert.h>
#include <curl/curl.h>

#define BAD_ACTOR_JSON_FMT                                                     \
	"{s:s,s:s,s:s,s:s,s:s,s:s,s:s,s:s,s:s,s:s,s:s,s:s,s:s}"
#define AUTH0_CLIENT_CREDS_JSON_FMT "{s:s,s:s,s:s,s:s}"
#define AUTH0_MAX_BEARER_TOKEN_LEN 4096
#define SENTRYPEER_USERAGENT "SentryPeer/1.0"

typedef struct memory_struct memory_struct;
struct memory_struct {
	char *memory;
	size_t size;
};

static size_t ignore_data(void *buffer, size_t size, size_t nmemb, void *userp)
{
	return size * nmemb;
}

static size_t save_json_results(void *json, size_t size, size_t nmemb,
				void *userp)
{
	size_t realsize = size * nmemb;
	memory_struct *mem = (memory_struct *)userp;

	char *ptr = realloc(mem->memory, mem->size + realsize + 1);
	assert(ptr); /* out of memory */

	mem->memory = ptr;
	memcpy(&(mem->memory[mem->size]), json, realsize);
	mem->size += realsize;
	mem->memory[mem->size] = 0;

	return realsize;
}

static void http_cleanup_curl(CURL *curl, struct curl_slist *headers)
{
	curl_easy_cleanup(curl);
	curl_slist_free_all(headers);

	// We are done with libcurl, so clean it up
	curl_global_cleanup();
}

static int request_oauth2_bearer_token(sentrypeer_config *config, CURL *curl)
{
	CURLcode res;
	memory_struct chunk;

	chunk.memory =
		malloc(1); /* Will be grown as needed by the realloc above */
	assert(chunk.memory);
	chunk.size = 0; /* no data at this point */

	/* Send all data to this function  */
	res = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, save_json_results);
	if (res != CURLE_OK) {
		fprintf(stderr, "curl_easy_setopt() failed: %s\n",
			curl_easy_strerror(res));
		free(chunk.memory);
		return EXIT_FAILURE;
	}

	/* We pass our 'chunk' struct to the callback function */
	res = curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
	if (res != CURLE_OK) {
		fprintf(stderr, "curl_easy_setopt() failed: %s\n",
			curl_easy_strerror(res));
		free(chunk.memory);
		return EXIT_FAILURE;
	}

	if (config->debug_mode || config->verbose_mode) {
		fprintf(stderr, "Requesting OAuth2 Bearer Token\n");
	}

	res = curl_easy_setopt(curl, CURLOPT_URL, SENTRYPEER_OAUTH2_TOKEN_URL);
	if (res != CURLE_OK) {
		fprintf(stderr, "curl_easy_setopt() failed: %s\n",
			curl_easy_strerror(res));
		free(chunk.memory);
		return EXIT_FAILURE;
	}

	json_error_t error;
	json_t *json_client_creds =
		json_pack(AUTH0_CLIENT_CREDS_JSON_FMT, "client_id",
			  config->oauth2_client_id, "client_secret",
			  config->oauth2_client_secret, "audience",
			  SENTRYPEER_OAUTH2_AUDIENCE, "grant_type",
			  SENTRYPEER_OAUTH2_GRANT_TYPE, &error);
	if (!json_client_creds) {
		fprintf(stderr,
			"Error creating json client credentials object: %s\n",
			error.text);
		free(chunk.memory);

		return EXIT_FAILURE;
	}

	char *json_string = json_dumps(json_client_creds, JSON_COMPACT);
	json_decref(json_client_creds);

	if (config->debug_mode || config->verbose_mode) {
		fprintf(stderr, "Client credentials in JSON format: %s\n",
			json_string);
	}

	res = curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_string);
	if (res != CURLE_OK) {
		fprintf(stderr, "curl_easy_setopt() failed: %s\n",
			curl_easy_strerror(res));
		free(json_string);
		free(chunk.memory);
		return EXIT_FAILURE;
	}

	res = curl_easy_perform(curl);
	if (res != CURLE_OK) {
		fprintf(stderr, "OAuth2 Token Request POSTing failed: %s\n",
			curl_easy_strerror(res));
		free(json_string);
		free(chunk.memory);
		return EXIT_FAILURE;
	}

	json_t *json_obj = json_loadb(chunk.memory, chunk.size, 0, NULL);
	assert(json_obj);

	json_t *access_token = json_object_get(json_obj, "access_token");
	assert(access_token);

	if (config->debug_mode || config->verbose_mode) {
		fprintf(stderr, "Got access_token: %s\n",
			json_string_value(access_token));
	}

	config->oauth2_access_token =
		util_duplicate_string(json_string_value(access_token));

	if (config->debug_mode || config->verbose_mode) {
		fprintf(stderr, "Retrieved access_token from config: %s\n",
			config->oauth2_access_token);
	}

	json_decref(json_obj);
	free(chunk.memory);
	free(json_string);

	return EXIT_SUCCESS;
}

static int set_oauth2_bearer_token_header(const sentrypeer_config *config,
					  CURL *curl,
					  struct curl_slist *headers)
{
	CURLcode res;

	size_t oauth2_bearer_header_len = strlen("Authorization: Bearer ") +
					  AUTH0_MAX_BEARER_TOKEN_LEN + 1;

	char *oauth2_bearer_header = malloc(oauth2_bearer_header_len);

	if (snprintf(oauth2_bearer_header, oauth2_bearer_header_len,
		     "Authorization: Bearer %s",
		     config->oauth2_access_token) < 0) {
		perror("snprintf() failed.");

		free(oauth2_bearer_header);
		return EXIT_FAILURE;
	}
	assert(oauth2_bearer_header);

	headers = curl_slist_append(
		headers, util_duplicate_string(oauth2_bearer_header));
	assert(headers);

	// Set the new HTTP headers
	res = curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
	if (res != CURLE_OK) {
		fprintf(stderr, "curl_easy_setopt() failed: %s\n",
			curl_easy_strerror(res));

		free(oauth2_bearer_header);

		return EXIT_FAILURE;
	}

	free(oauth2_bearer_header);

	return EXIT_SUCCESS;
}

static int get_and_set_oauth2_bearer_token(sentrypeer_config *config,
					   CURL *curl,
					   struct curl_slist *headers)
{
	if (request_oauth2_bearer_token(config, curl) != EXIT_SUCCESS) {
		fprintf(stderr, "Failed to get OAuth2 Bearer token.\n");
		return EXIT_FAILURE;
	}

	if (set_oauth2_bearer_token_header(config, curl, headers) !=
	    EXIT_SUCCESS) {
		fprintf(stderr, "Failed to set OAuth2 Bearer token header.\n");
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

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

int json_http_post_bad_actor(sentrypeer_config *config,
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

	struct curl_slist *headers = 0;
	headers = curl_slist_append(headers, "Content-Type: application/json");

	char *json_string =
		bad_actor_to_json(config,
				  bad_actor_to_log); // Caller must free
	if (json_string == NULL) {
		fprintf(stderr, "Failed to convert bad actor to json.\n");

		free(json_string);
		http_cleanup_curl(curl, headers);

		return EXIT_FAILURE;
	}

	res = curl_easy_setopt(curl, CURLOPT_USERAGENT, SENTRYPEER_USERAGENT);
	if (res != CURLE_OK) {
		fprintf(stderr, "curl_easy_setopt() failed: %s\n",
			curl_easy_strerror(res));

		free(json_string);
		http_cleanup_curl(curl, headers);

		return EXIT_FAILURE;
	}

	// Enables TLSv1.2 / TLSv1.3 version only
	res = curl_easy_setopt(curl, CURLOPT_SSLVERSION,
			       CURL_SSLVERSION_TLSv1_2);
	if (res != CURLE_OK) {
		fprintf(stderr, "curl_easy_setopt() failed: %s\n",
			curl_easy_strerror(res));

		free(json_string);
		http_cleanup_curl(curl, headers);

		return EXIT_FAILURE;
	}

	// Complete within 2 seconds
	res = curl_easy_setopt(curl, CURLOPT_TIMEOUT, 2L);
	if (res != CURLE_OK) {
		fprintf(stderr, "curl_easy_setopt() failed: %s\n",
			curl_easy_strerror(res));

		free(json_string);
		http_cleanup_curl(curl, headers);

		return EXIT_FAILURE;
	}

	// We already have an access token, so we set it in our header
	if (config->oauth2_mode) {
		if (config->oauth2_access_token != 0) {
			if (set_oauth2_bearer_token_header(
				    config, curl, headers) != EXIT_SUCCESS) {
				fprintf(stderr,
					"Failed to set OAuth2 Bearer token header.\n");

				free(json_string);
				http_cleanup_curl(curl, headers);

				return EXIT_FAILURE;
			}
		} else {
			// Set our normal headers
			res = curl_easy_setopt(curl, CURLOPT_HTTPHEADER,
					       headers);
			if (res != CURLE_OK) {
				fprintf(stderr,
					"curl_easy_setopt() failed: %s\n",
					curl_easy_strerror(res));

				free(json_string);
				http_cleanup_curl(curl, headers);

				return EXIT_FAILURE;
			}
		}
	}

	if (config->oauth2_mode && config->oauth2_access_token == 0) {
		if (get_and_set_oauth2_bearer_token(config, curl, headers) !=
		    EXIT_SUCCESS) {
			fprintf(stderr,
				"Failed to get and set OAuth2 Bearer token.\n");

			free(json_string);
			http_cleanup_curl(curl, headers);

			return EXIT_FAILURE;
		}
	}

	// Reset the JSON payload as it might have been changed by get_and_set_oauth2_bearer_token
	res = curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_string);
	if (res != CURLE_OK) {
		fprintf(stderr, "curl_easy_setopt() failed: %s\n",
			curl_easy_strerror(res));

		free(json_string);
		http_cleanup_curl(curl, headers);

		return EXIT_FAILURE;
	}

	// Reset the URL as it might have been changed by get_and_set_oauth2_bearer_token
	res = curl_easy_setopt(curl, CURLOPT_URL, config->webhook_url);
	if (res != CURLE_OK) {
		fprintf(stderr, "curl_easy_setopt() failed: %s\n",
			curl_easy_strerror(res));

		free(json_string);
		http_cleanup_curl(curl, headers);

		return EXIT_FAILURE;
	}

	// Stop writing to stdout
	res = curl_easy_setopt(curl, CURLOPT_WRITEDATA, NULL);
	if (res != CURLE_OK) {
		fprintf(stderr, "curl_easy_setopt() failed: %s\n",
			curl_easy_strerror(res));

		free(json_string);
		http_cleanup_curl(curl, headers);

		return EXIT_FAILURE;
	}

	res = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, ignore_data);
	if (res != CURLE_OK) {
		fprintf(stderr, "curl_easy_setopt() failed: %s\n",
			curl_easy_strerror(res));

		free(json_string);
		http_cleanup_curl(curl, headers);

		return EXIT_FAILURE;
	}

	res = curl_easy_perform(curl);
	if (res != CURLE_OK) {
		fprintf(stderr, "WebHook POSTing failed: %d, %s\n", res,
			curl_easy_strerror(res));

		free(json_string);
		http_cleanup_curl(curl, headers);

		return EXIT_FAILURE;
	}

	long http_response_code = 0;
	curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_response_code);
	if (http_response_code != 200 && http_response_code != 201) {
		if (config->oauth2_mode) {
			if (http_response_code == 401 ||
			    http_response_code == 403) {
				// The token has probably expired (lasts 86400 seconds - 1 day)
				// Let's reset it and get a new one
				if (config->oauth2_access_token != 0) {
					free(config->oauth2_access_token);
					config->oauth2_access_token = 0;
				}
				if (json_http_post_bad_actor(
					    config, bad_actor_to_log)) {
					fprintf(stderr,
						"Failed to POST bad actor.\n");

					return EXIT_FAILURE;
				}
			} else {
				fprintf(stderr,
					"WebHook POSTing failed: HTTP response code %ld\n",
					http_response_code);

				free(json_string);
				http_cleanup_curl(curl, headers);

				return EXIT_FAILURE;
			}
		} else {
			fprintf(stderr,
				"WebHook POSTing failed: HTTP response code %ld\n",
				http_response_code);

			free(json_string);
			http_cleanup_curl(curl, headers);

			return EXIT_FAILURE;
		}
	}

	free(json_string);
	http_cleanup_curl(curl, headers);

	if (config->debug_mode || config->verbose_mode) {
		fprintf(stderr,
			"WebHook POSTing succeeded: HTTP response code %ld\n",
			http_response_code);
	}
	return EXIT_SUCCESS;
}
