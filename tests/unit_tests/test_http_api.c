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

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <stdint.h>
#include <cmocka.h>

#include "test_http_api.h"
#include "../../src/http_routes.h"
#include "../../src/http_daemon.h"
#include <stdio.h>
#include <stdlib.h>
#include <curl/curl.h>
#include <jansson.h>
#include <config.h>

static size_t curl_to_jansson_to_version(void *buffer, size_t size,
					 size_t nmemb, void *userp)
{
	json_t *json = json_loadb(buffer, size * nmemb, 0, NULL);
	assert_non_null(json);

	json_t *sentrypeer_version_json = json_object_get(json, "version");
	assert_non_null(sentrypeer_version_json);
	fprintf(stderr, "SentryPeer version from RESTful API: %s\n",
		json_string_value(sentrypeer_version_json));
	assert_string_equal(json_string_value(sentrypeer_version_json),
			    PACKAGE_VERSION);

	*((json_t **)userp) = json;
	json_decref(json);

	return size * nmemb;
}

int test_http_api_health_check_version(void)
{
	CURL *curl = curl_easy_init();
	assert_non_null(curl);

	curl_easy_setopt(curl, CURLOPT_URL,
			 "http://localhost:8082/health-check");

	struct curl_slist *headers = NULL;
	headers = curl_slist_append(headers, "Content-Type: application/json");

	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION,
			 curl_to_jansson_to_version);

	assert_int_equal(curl_easy_perform(curl), CURLE_OK);

	curl_easy_cleanup(curl);
	curl_slist_free_all(headers);

	return EXIT_SUCCESS;
}

// Returns response code from curl
static long curl_get_url(const char *url)
{
	CURL *curl;
	CURLcode res;

	long http_response_code = 0;

	curl = curl_easy_init();
	if (!curl) {
		fprintf(stderr, "curl_easy_init() failed\n");
		return CURLE_FAILED_INIT;
	}

	curl_easy_setopt(curl, CURLOPT_URL, url);
	res = curl_easy_perform(curl);
	if (res != CURLE_OK) {
		fprintf(stderr, "curl_easy_perform() failed: %s\n",
			curl_easy_strerror(res));
		return EXIT_FAILURE;
	}

	curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_response_code);
	curl_easy_cleanup(curl);

	return http_response_code;
}

void test_http_api_get(void **state)
{
	sentrypeer_config *config = *state;
	assert_non_null(config);

	assert_int_equal(curl_global_init(CURL_GLOBAL_ALL), CURLE_OK);

	config->debug_mode = true;
	fprintf(stderr, "Debug mode set to true at line number %d in file %s\n",
		__LINE__ - 1, __FILE__);

	assert_int_equal(http_daemon_init(config), EXIT_SUCCESS);
	fprintf(stderr, "http_daemon started at line number %d in file %s\n",
		__LINE__ - 1, __FILE__);

	assert_int_equal(test_http_api_health_check_version(), EXIT_SUCCESS);
	fprintf(stderr,
		"test_http_api_health_check_version res at line number %d in file %s\n",
		__LINE__ - 1, __FILE__);

	// Health Check. This is 200 OK
	assert_int_equal(curl_get_url("http://localhost:8082/health-check"),
			 200);

	// This is 404 Not Found
	assert_int_equal(curl_get_url("http://127.0.0.1:8082/blah"), 404);

	// Bad actor 404 Not Found
	assert_int_equal(
		curl_get_url("http://127.0.0.1:8082/ip-addresses/8.8.8.8"),
		404);

	// Bad actor check 200 OK (switch to BAD_ACTOR_SOURCE_IP from test_database.h)
	assert_int_equal(
		curl_get_url(
			"http://127.0.0.1:8082/ip-addresses/104.149.141.214"),
		200);

	// Bad actor 400 Bad Data
	assert_int_equal(
		curl_get_url(
			"http://127.0.0.1:8082/ip-addresses/104.14da3afcsasd"),
		400);

	curl_global_cleanup();
}

void test_route_regex_check(void **state)
{
	(void)state; /* unused */

	char *matched_string = 0;
	sentrypeer_config *config = sentrypeer_config_new();
	config->debug_mode = true;
	assert_null(matched_string);
	assert_int_equal(route_regex_check("/ip-addresses/8.8.8.8",
					   IP_ADDRESS_ROUTE, &matched_string,
					   config),
			 0);
	assert_non_null(matched_string);
	assert_string_equal(matched_string, "8.8.8.8");

	sentrypeer_config_destroy(&config);

	// Check a double-free for fun.
	free(matched_string);
	matched_string = 0;
	free(matched_string);
}
