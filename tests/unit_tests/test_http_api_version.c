/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only  */
/* Copyright (c) 2021 - 2024 Gavin Henry <ghenry@sentrypeer.org> */
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

#include <string.h>
#include <stdlib.h>
#include <jansson.h>
#include <curl/curl.h>
#include <config.h>

static size_t save_json_results(void *json, size_t size, size_t nmemb,
				void *userp)
{
	size_t realsize = size * nmemb;
	memory_struct *mem = (memory_struct *)userp;

	char *ptr = realloc(mem->memory, mem->size + realsize + 1);
	assert_non_null(ptr); /* out of memory */

	mem->memory = ptr;
	memcpy(&(mem->memory[mem->size]), json, realsize);
	mem->size += realsize;
	mem->memory[mem->size] = 0;

	return realsize;
}

int test_http_api_health_check_version(void)
{
	CURL *curl_handle;
	memory_struct chunk;

	chunk.memory =
		malloc(1); /* Will be grown as needed by the realloc above */
	assert_non_null(chunk.memory);
	chunk.size = 0; /* no data at this point */

	curl_global_init(CURL_GLOBAL_ALL);

	/* Init the curl session */
	curl_handle = curl_easy_init();
	assert_non_null(curl_handle);

	/* Specify URL to get */
	curl_easy_setopt(curl_handle, CURLOPT_URL,
			 "http://localhost:8082/health-check");

	/* Send all data to this function  */
	curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, save_json_results);

	/* We pass our 'chunk' struct to the callback function */
	curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);

	struct curl_slist *headers = 0;
	headers = curl_slist_append(headers, "Content-Type: application/json");
	curl_easy_setopt(curl_handle, CURLOPT_HTTPHEADER, headers);

	/* Some servers do not like requests that are made without a user-agent
     	   field, so we provide one */
	curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "SentryPeer/1.0");

	/* Save version output for troubleshooting in test-suite.log if failures */
	curl_easy_setopt(curl_handle, CURLOPT_VERBOSE, 1L);

	/* Get it! */
	assert_int_equal(curl_easy_perform(curl_handle), CURLE_OK);

	json_t *json_obj = json_loadb(chunk.memory, chunk.size, 0, NULL);
	assert_non_null(json_obj);

	json_t *sentrypeer_version_json = json_object_get(json_obj, "version");
	assert_non_null(sentrypeer_version_json);

	fprintf(stderr, "SentryPeer version from RESTful API: %s\n",
		json_string_value(sentrypeer_version_json));

	assert_string_equal(json_string_value(sentrypeer_version_json),
			    PACKAGE_VERSION);

	/* Cleanup curl stuff */
	curl_easy_cleanup(curl_handle);
	curl_slist_free_all(headers);

	json_decref(json_obj);
	free(chunk.memory);

	/* We are done with libcurl, so clean it up */
	curl_global_cleanup();

	return EXIT_SUCCESS;
}
