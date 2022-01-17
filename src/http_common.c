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

#include "http_common.h"
#include "config.h"

#include <stdbool.h>
#include <arpa/inet.h>
#include <string.h>
#include <strings.h>
#include <stdio.h>
#include <stdlib.h>

void log_http_client_ip(const char *url, struct MHD_Connection *connection)
{
	const struct sockaddr *addr =
		MHD_get_connection_info(connection,
					MHD_CONNECTION_INFO_CLIENT_ADDRESS)
			->client_addr;

	if (addr != NULL) {
		char client_ip_str[INET6_ADDRSTRLEN];
		inet_ntop(addr->sa_family,
			  addr->sa_family == AF_INET ?
				  (void *)&(((struct sockaddr_in *)addr)
						    ->sin_addr) :
					(void *)&(((struct sockaddr_in6 *)addr)
						    ->sin6_addr),
			  client_ip_str, sizeof(client_ip_str));

		fprintf(stderr, "GET %s from Client IP: %s\n", url,
			client_ip_str);
	} else {
		fprintf(stderr, "GET %s from Client IP: Unknown\n", url);
	}
}

bool json_is_requested(struct MHD_Connection *connection)
{
	if (MHD_lookup_connection_value(connection, MHD_HEADER_KIND,
					MHD_HTTP_HEADER_CONTENT_TYPE) != NULL) {
		fprintf(stderr, "Content-Type is: %s\n",
			MHD_lookup_connection_value(
				connection, MHD_HEADER_KIND,
				MHD_HTTP_HEADER_CONTENT_TYPE));

		if (strncasecmp(MHD_lookup_connection_value(
					connection, MHD_HEADER_KIND,
					MHD_HTTP_HEADER_CONTENT_TYPE),
				CONTENT_TYPE_JSON,
				strlen(CONTENT_TYPE_JSON)) == 0) {
			return true;
		} else {
			return false;
		}
	} else {
		return false;
	}
}

int finalise_response(struct MHD_Connection *connection, const char *reply_data,
		      const char *content_type, int status_code,
		      bool free_reply_data)
{
	enum MHD_ResponseMemoryMode memory_mode = MHD_RESPMEM_PERSISTENT;
	if (free_reply_data) {
		memory_mode = MHD_RESPMEM_MUST_FREE;
	}

	struct MHD_Response *response = MHD_create_response_from_buffer(
		strlen(reply_data), (void *)reply_data, memory_mode);

	if (NULL == response)
		return MHD_NO;
	int ret = MHD_queue_response(connection, status_code, response);
	if (MHD_add_response_header(response, MHD_HTTP_HEADER_CONTENT_TYPE,
				    content_type) == MHD_NO) {
		fprintf(stderr, "Failed to add header\n");
		MHD_destroy_response(response);
		return MHD_NO;
	}

	// CORS - https://developer.mozilla.org/en-US/docs/Web/HTTP/CORS/Errors/CORSMissingAllowOrigin
	if (MHD_add_response_header(response, "Access-Control-Allow-Origin",
				    "*") == MHD_NO) {
		fprintf(stderr, "Failed to add header\n");
		MHD_destroy_response(response);
		return MHD_NO;
	}

	if (MHD_add_response_header(response, "X-Powered-By", "SentryPeer") ==
	    MHD_NO) {
		fprintf(stderr, "Failed to add header\n");
		MHD_destroy_response(response);
		return MHD_NO;
	}

	if (MHD_add_response_header(response, "X-SentryPeer-Version",
				    PACKAGE_VERSION) == MHD_NO) {
		fprintf(stderr, "Failed to add header\n");
		MHD_destroy_response(response);
		return MHD_NO;
	}

	MHD_destroy_response(response);
	return ret;
}
