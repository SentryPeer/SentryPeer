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
#include "../../src/http_daemon.h"
#include <stdio.h>
#include <stdlib.h>
#include <curl/curl.h>

void test_http_api_get(void **state)
{
	(void)state; /* unused */

	struct sentrypeer_config config;
	config.debug_mode = true;
	fprintf(stderr, "Debug mode set to true at line number %d in file %s\n",
		__LINE__ - 1, __FILE__);

	assert_int_equal(http_daemon_init(&config), EXIT_SUCCESS);
	fprintf(stderr, "http_daemon started at line number %d in file %s\n",
		__LINE__ - 1, __FILE__);

	CURL *curl = curl_easy_init();
	assert_non_null(curl);
	if (curl) {
		curl_easy_setopt(curl, CURLOPT_URL, "http://127.0.0.1:8082");
		assert_int_equal(curl_easy_perform(curl), CURLE_OK);
		curl_easy_cleanup(curl);
	}
}
