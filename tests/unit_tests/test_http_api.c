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
#include "test_http_api_version.h"
#include "../../src/http_routes.h"
#include "../../src/http_daemon.h"
#include <stdio.h>
#include <stdlib.h>
#include <curl/curl.h>

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

	fprintf(stderr, "Debug mode set to true at line number %d in file %s\n",
		__LINE__ - 1, __FILE__);

	assert_int_equal(http_daemon_init(config), EXIT_SUCCESS);
	fprintf(stderr, "http_daemon started at line number %d in file %s\n",
		__LINE__ - 1, __FILE__);

	assert_int_equal(test_http_api_health_check_version(), EXIT_SUCCESS);

	// Health Check. This is 200 OK
	assert_int_equal(curl_get_url("http://localhost:8082/health-check"),
			 200);

	// This is 404 Not Found
	assert_int_equal(curl_get_url("http://127.0.0.1:8082/blah"), 404);

	// Bad actor 404 Not Found
	assert_int_equal(
		curl_get_url("http://127.0.0.1:8082/ip-addresses/8.8.8.8"),
		404);

	// Bad actor check 200 OK (switch to BAD_ACTOR_SOURCE_IP from test_database.c)
	assert_int_equal(
		curl_get_url(
			"http://127.0.0.1:8082/ip-addresses/104.149.141.214"),
		200);

	// Bad actors check 200 OK
	assert_int_equal(curl_get_url("http://127.0.0.1:8082/ip-addresses"),
			 200);

	// Bad actor 400 Bad Data
	assert_int_equal(
		curl_get_url(
			"http://127.0.0.1:8082/ip-addresses/104.14da3afcsasd"),
		400);

	// Phone numbers check 200 OK
	assert_int_equal(curl_get_url("http://127.0.0.1:8082/numbers"), 200);

	// Phone number 404 Not Found
	assert_int_equal(
		curl_get_url("http://127.0.0.1:8082/numbers/123456789"), 404);

	// Phone number 200 OK
	assert_int_equal(curl_get_url("http://127.0.0.1:8082/numbers/100"),
			 200);

	assert_int_equal(http_daemon_stop(config), EXIT_SUCCESS);
	fprintf(stderr, "http_daemon stopped at line number %d in file %s\n",
		__LINE__ - 1, __FILE__);

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
	assert_null(config);

	// Check a double-free for fun.
	free(matched_string);
	matched_string = 0;
	free(matched_string);
}
