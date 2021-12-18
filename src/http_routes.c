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

#include <jansson.h>
#include <config.h>
#include <string.h>

#include "http_routes.h"
#include "http_common.h"
#include "conf.h"

#define NOT_FOUND_ERROR                                                        \
	"<html><head><title>404 Not found</title></head><body><h1>404 Error</h1><h2>The requested resource could not be found.</h2></body></html>"

enum MHD_Result route_handler(void *cls, struct MHD_Connection *connection,
			       const char *url, const char *method,
			       const char *version, const char *upload_data,
			       size_t *upload_data_size, void **ptr)
{
	static int dummy;
	sentrypeer_config *config = (sentrypeer_config *)cls;

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

	// TODO: Switch to a dispatch table or similar later if more routes are added
	if (0 == strncmp(url, HEALTH_CHECK_ROUTE, HTTP_ROUTES_MAX_LEN)) {
		return health_check_route(connection);
	} else if (0 == strncmp(url, HOME_PAGE_ROUTE, HTTP_ROUTES_MAX_LEN)) {
		return finalise_response(connection, HOME_PAGE_ROUTE,
					 CONTENT_TYPE_HTML, MHD_HTTP_OK);
	} else if (0 == strncmp(url, IP_ADDRESSES_ROUTE, HTTP_ROUTES_MAX_LEN)) {
		return ip_addresses_route(connection, config);
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