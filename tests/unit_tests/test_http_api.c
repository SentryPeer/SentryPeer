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

size_t curl_to_jansson_to_version(void *buffer, size_t size, size_t nmemb,
				  void *userp)
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
	return size * nmemb;
}

void test_http_api_get(void **state)
{
	sentrypeer_config *config = *state;
	assert_non_null(config);

	config->debug_mode = true;
	fprintf(stderr, "Debug mode set to true at line number %d in file %s\n",
		__LINE__ - 1, __FILE__);

	assert_int_equal(http_daemon_init(config), EXIT_SUCCESS);
	fprintf(stderr, "http_daemon started at line number %d in file %s\n",
		__LINE__ - 1, __FILE__);

	curl_global_init(CURL_GLOBAL_ALL);
	CURL *easyhandle = curl_easy_init();
	assert_non_null(easyhandle);
	if (easyhandle) {
		// Health Check. This is 200 OK
		curl_easy_setopt(easyhandle, CURLOPT_URL,
				 "http://127.0.0.1:8082/health-check");
		struct curl_slist *headers = NULL;
		headers = curl_slist_append(headers,
					    "Content-Type: application/json");
		curl_easy_setopt(easyhandle, CURLOPT_HTTPHEADER, headers);
		curl_easy_setopt(easyhandle, CURLOPT_WRITEFUNCTION,
				 curl_to_jansson_to_version);
		assert_int_equal(curl_easy_perform(easyhandle), CURLE_OK);

		long http_response_code = 0;
		curl_easy_getinfo(easyhandle, CURLINFO_RESPONSE_CODE,
				  &http_response_code);
		fprintf(stderr,
			"Response code for 200 test at line number %d in file %s is: %ld\n",
			__LINE__ - 1, __FILE__, http_response_code);
		assert_int_equal(http_response_code, 200);

		// This is 404 Not Found
		curl_easy_setopt(easyhandle, CURLOPT_URL,
				 "http://127.0.0.1:8082/blah");
		curl_easy_setopt(easyhandle, CURLOPT_WRITEFUNCTION, NULL);
		assert_int_equal(curl_easy_perform(easyhandle), CURLE_OK);
		curl_easy_getinfo(easyhandle, CURLINFO_RESPONSE_CODE,
				  &http_response_code);
		fprintf(stderr,
			"Response code for 404 test at line number %d in file %s is: %ld\n",
			__LINE__ - 1, __FILE__, http_response_code);
		assert_int_equal(http_response_code, 404);

		// Bad actor 404 Not Found
		curl_easy_setopt(easyhandle, CURLOPT_URL,
				 "http://127.0.0.1:8082/ip-addresses/8.8.8.8");
		curl_easy_setopt(easyhandle, CURLOPT_WRITEFUNCTION, NULL);
		assert_int_equal(curl_easy_perform(easyhandle), CURLE_OK);
		curl_easy_getinfo(easyhandle, CURLINFO_RESPONSE_CODE,
				  &http_response_code);
		fprintf(stderr,
			"Response code for 404 test at line number %d in file %s is: %ld\n",
			__LINE__ - 1, __FILE__, http_response_code);
		assert_int_equal(http_response_code, 404);

		// Bad actor check 200 OK (switch to BAD_ACTOR_SOURCE_IP from test_database.h)
		curl_easy_setopt(
			easyhandle, CURLOPT_URL,
			"http://127.0.0.1:8082/ip-addresses/104.149.141.214");
		curl_easy_setopt(easyhandle, CURLOPT_WRITEFUNCTION, NULL);
		assert_int_equal(curl_easy_perform(easyhandle), CURLE_OK);
		curl_easy_getinfo(easyhandle, CURLINFO_RESPONSE_CODE,
				  &http_response_code);
		fprintf(stderr,
			"Response code for 200 test at line number %d in file %s is: %ld\n",
			__LINE__ - 1, __FILE__, http_response_code);
		assert_int_equal(http_response_code, 200);

		curl_slist_free_all(headers);
		curl_easy_cleanup(easyhandle);
		curl_global_cleanup();
	}
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
