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

#include "conf.h"
#include "http_daemon.h"
#include "http_common.h"
#include "http_routes.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <microhttpd.h>
#include <jansson.h>
#include <config.h>

#define NOT_FOUND_ERROR                                                        \
	"<html><head><title>404 Not found</title></head><body><h1>404 Error</h1><h2>The requested resource could not be found.</h2></body></html>"

static int health_check_route(struct MHD_Connection *connection)
{
	const char *reply_to_get = 0;
	const char *html_text =
		"<html><body><h1>Hello from SentryPeer!</h1><h2>All is well!</h2></body></html>";
	const char *content_type = 0;

	if (json_is_requested(connection)) {
		json_t *api_reply_to_get_json =
			json_pack("{s:s, s:s, s:s}", "status", "OK", "message",
				  "Hello from SentryPeer!", "version",
				  PACKAGE_VERSION);
		reply_to_get =
			json_dumps(api_reply_to_get_json, JSON_INDENT(2));
		content_type = CONTENT_TYPE_JSON;
		// Free the json object
		json_decref(api_reply_to_get_json);
	} else {
		content_type = CONTENT_TYPE_HTML;
		reply_to_get = html_text;
	}

	return finalise_response(connection, reply_to_get, content_type,
				 MHD_HTTP_OK);
}

static enum MHD_Result ahc_get(void *cls, struct MHD_Connection *connection,
			       const char *url, const char *method,
			       const char *version, const char *upload_data,
			       size_t *upload_data_size, void **ptr)
{
	static int dummy;

	if (strcmp(method, MHD_HTTP_METHOD_GET) != 0)
		return MHD_NO; /* unexpected method */

	if (&dummy != *ptr) {
		/* The first time only the headers are valid,
         do not respond in the first round... */
		*ptr = &dummy;
		return MHD_YES;
	}

	if (0 != *upload_data_size)
		return MHD_NO; /* upload data in a GET!? */
	*ptr = NULL; /* clear context pointer */

	log_http_client_ip(url, connection);

	if (0 == strncmp(url, HEALTH_CHECK_ROUTE, HTTP_ROUTES_MAX_LEN)) {
		return health_check_route(connection);
	} else if (0 == strncmp(url, HOME_PAGE_ROUTE, HTTP_ROUTES_MAX_LEN)) {
		return finalise_response(connection, HOME_PAGE_ROUTE,
					 CONTENT_TYPE_HTML, MHD_HTTP_OK);
	} else if (0 == strncmp(url, IP_ADDRESSES_ROUTE, HTTP_ROUTES_MAX_LEN)) {
		return finalise_response(connection, IP_ADDRESSES_ROUTE,
					 CONTENT_TYPE_HTML, MHD_HTTP_OK);
	} else if (0 == strncmp(url, IP_ADDRESS_ROUTE, HTTP_ROUTES_MAX_LEN)) {
		return finalise_response(connection, IP_ADDRESS_ROUTE,
					 CONTENT_TYPE_HTML, MHD_HTTP_OK);
	} else if (0 == strncmp(url, IP_ADDRESSES_IPSET_ROUTE,
				HTTP_ROUTES_MAX_LEN)) {
		return finalise_response(connection, IP_ADDRESSES_IPSET_ROUTE,
					 CONTENT_TYPE_HTML, MHD_HTTP_OK);
	} else if (0 == strncmp(url, NUMBERS_ROUTE, HTTP_ROUTES_MAX_LEN)) {
		return finalise_response(connection, NUMBERS_ROUTE,
					 CONTENT_TYPE_HTML, MHD_HTTP_OK);
	} else if (0 == strncmp(url, NUMBER_ROUTE, HTTP_ROUTES_MAX_LEN)) {
		return finalise_response(connection, NUMBER_ROUTE,
					 CONTENT_TYPE_HTML, MHD_HTTP_OK);
	} else if (0 == strncmp(url, COUNTRIES_ROUTE, HTTP_ROUTES_MAX_LEN)) {
		return finalise_response(connection, COUNTRIES_ROUTE,
					 CONTENT_TYPE_HTML, MHD_HTTP_OK);
	} else if (0 == strncmp(url, COUNTRY_ROUTE, HTTP_ROUTES_MAX_LEN)) {
		return finalise_response(connection, COUNTRY_ROUTE,
					 CONTENT_TYPE_HTML, MHD_HTTP_OK);
	} else if (0 == strncmp(url, COUNTRY_CITY_ROUTE, HTTP_ROUTES_MAX_LEN)) {
		return finalise_response(connection, COUNTRY_CITY_ROUTE,
					 CONTENT_TYPE_HTML, MHD_HTTP_OK);
	} else if (0 == strncmp(url, USER_AGENTS_ROUTE, HTTP_ROUTES_MAX_LEN)) {
		return finalise_response(connection, USER_AGENTS_ROUTE,
					 CONTENT_TYPE_HTML, MHD_HTTP_OK);
	} else if (0 == strncmp(url, USER_AGENT_ROUTE, HTTP_ROUTES_MAX_LEN)) {
		return finalise_response(connection, USER_AGENT_ROUTE,
					 CONTENT_TYPE_HTML, MHD_HTTP_OK);
	} else if (0 == strncmp(url, SIP_METHODS_ROUTE, HTTP_ROUTES_MAX_LEN)) {
		return finalise_response(connection, SIP_METHODS_ROUTE,
					 CONTENT_TYPE_HTML, MHD_HTTP_OK);
	} else if (0 == strncmp(url, SIP_METHOD_ROUTE, HTTP_ROUTES_MAX_LEN)) {
		return finalise_response(connection, SIP_METHOD_ROUTE,
					 CONTENT_TYPE_HTML, MHD_HTTP_OK);
	} else {
		return finalise_response(connection, NOT_FOUND_ERROR,
					 CONTENT_TYPE_HTML, MHD_HTTP_NOT_FOUND);
	}
}

int http_daemon_init(sentrypeer_config const *config)
{
	if (config->debug_mode || config->verbose_mode) {
		fprintf(stderr, "Starting http daemon...\n");
	}

	struct MHD_Daemon *daemon;

	daemon = MHD_start_daemon(MHD_USE_THREAD_PER_CONNECTION,
				  HTTP_DAEMON_PORT, NULL, NULL, &ahc_get, NULL,
				  MHD_OPTION_END);
	if (daemon == NULL) {
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;

	// MHD_stop_daemon(d);
}
