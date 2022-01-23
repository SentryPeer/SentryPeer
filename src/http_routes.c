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

#include <jansson.h>
#include "config.h"
#include <string.h>

#include "http_routes.h"
#include "http_common.h"
#include "conf.h"
#include "utils.h"

int route_check(const char *url, const char *route,
		sentrypeer_config const *config)
{
	// We don't want any partial matches (e.g. "/ip-" matches "/ip-address")
	// or "/ip-address/8.8.8.8" matches "/ip-address".
	if (config->debug_mode || config->verbose_mode) {
		fprintf(stderr, "Checking url: %s, against route: %s\n", url,
			route);
	}
	if (strcmp(url, route) == 0) {
		// The route is an exact match.
		if (config->debug_mode || config->verbose_mode) {
			fprintf(stderr, "Route is an exact match.\n");
		}
		return EXIT_SUCCESS;
	} else {
		// The route is not a match.
		return EXIT_FAILURE;
	}
}

enum MHD_Result route_handler(void *cls, struct MHD_Connection *connection,
			      const char *url, const char *method,
			      const char *version, const char *upload_data,
			      size_t *upload_data_size, void **ptr)
{
	static int dummy;
	sentrypeer_config const *config = (sentrypeer_config *)cls;

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
	char *matched_ip_address = 0;
	char *matched_phone_number = 0;

	// TODO: Switch to a dispatch table or similar later as starting to hurt eyes.
	if (route_check(url, HEALTH_CHECK_ROUTE, config) == EXIT_SUCCESS) {
		return health_check_route(connection);
	} else if (route_check(url, HOME_PAGE_ROUTE, config) == EXIT_SUCCESS) {
		if (config->web_gui_mode) {
			return finalise_response(
				connection, "Vue.js SPA will be served here.",
				CONTENT_TYPE_HTML, MHD_HTTP_OK, false);
		} else {
			return finalise_response(connection,
						 NOT_FOUND_ERROR_JSON,
						 CONTENT_TYPE_JSON,
						 MHD_HTTP_NOT_FOUND, false);
		}

	} else if (route_check(url, IP_ADDRESSES_ROUTE, config) ==
		   EXIT_SUCCESS) {
		return ip_addresses_route(connection, config);
	} else if (route_regex_check(url, IP_ADDRESS_ROUTE, &matched_ip_address,
				     config) == EXIT_SUCCESS) {
		if (config->debug_mode || config->verbose_mode) {
			fprintf(stderr, "Matched ip address route: %s\n",
				IP_ADDRESS_ROUTE);
		}

		if (valid_ip_address_format(matched_ip_address) ==
		    EXIT_SUCCESS) {
			return ip_address_route(&matched_ip_address, connection,
						config);
		} else {
			free(matched_ip_address);
			matched_ip_address = 0;
			return finalise_response(connection, BAD_DATA_JSON,
						 CONTENT_TYPE_JSON,
						 MHD_HTTP_BAD_REQUEST, false);
		}
	} else if (route_check(url, IP_ADDRESSES_IPSET_ROUTE, config) ==
		   EXIT_SUCCESS) {
		return finalise_response(connection, IP_ADDRESSES_IPSET_ROUTE,
					 CONTENT_TYPE_HTML, MHD_HTTP_OK, false);
	} else if (route_check(url, NUMBERS_ROUTE, config) == EXIT_SUCCESS) {
		return called_numbers_route(connection, config);
	} else if (route_regex_check(url, NUMBER_ROUTE, &matched_phone_number,
				     config) == EXIT_SUCCESS) {
		if (config->debug_mode || config->verbose_mode) {
			fprintf(stderr, "Called Number route: %s\n",
				NUMBER_ROUTE);
		}

		return called_number_route(&matched_phone_number, connection,
					   config);
	} else if (route_check(url, COUNTRIES_ROUTE, config) == EXIT_SUCCESS) {
		return finalise_response(connection, COUNTRIES_ROUTE,
					 CONTENT_TYPE_HTML, MHD_HTTP_OK, false);
	} else if (route_check(url, COUNTRY_ROUTE, config) == EXIT_SUCCESS) {
		return finalise_response(connection, COUNTRY_ROUTE,
					 CONTENT_TYPE_HTML, MHD_HTTP_OK, false);
	} else if (route_check(url, COUNTRY_CITY_ROUTE, config) ==
		   EXIT_SUCCESS) {
		return finalise_response(connection, COUNTRY_CITY_ROUTE,
					 CONTENT_TYPE_HTML, MHD_HTTP_OK, false);
	} else if (route_check(url, USER_AGENTS_ROUTE, config) ==
		   EXIT_SUCCESS) {
		return finalise_response(connection, USER_AGENTS_ROUTE,
					 CONTENT_TYPE_HTML, MHD_HTTP_OK, false);
	} else if (route_check(url, USER_AGENT_ROUTE, config) == EXIT_SUCCESS) {
		return finalise_response(connection, USER_AGENT_ROUTE,
					 CONTENT_TYPE_HTML, MHD_HTTP_OK, false);
	} else if (route_check(url, SIP_METHODS_ROUTE, config) ==
		   EXIT_SUCCESS) {
		return finalise_response(connection, SIP_METHODS_ROUTE,
					 CONTENT_TYPE_HTML, MHD_HTTP_OK, false);
	} else if (route_check(url, SIP_METHOD_ROUTE, config) == EXIT_SUCCESS) {
		return finalise_response(connection, SIP_METHOD_ROUTE,
					 CONTENT_TYPE_HTML, MHD_HTTP_OK, false);
	} else {
		if (config->debug_mode || config->verbose_mode) {
			fprintf(stderr, "No route matched.\n");
		}
		return finalise_response(connection, NOT_FOUND_ERROR_JSON,
					 CONTENT_TYPE_JSON, MHD_HTTP_NOT_FOUND,
					 false);
	}
}
