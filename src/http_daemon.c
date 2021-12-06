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

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <strings.h>
#include <stdlib.h>
#include <arpa/inet.h>

#include <microhttpd.h>
#include <jansson.h>
#include <config.h>

static enum MHD_Result ahc_get(void *cls, struct MHD_Connection *connection,
			       const char *url, const char *method,
			       const char *version, const char *upload_data,
			       size_t *upload_data_size, void **ptr)
{
	static int dummy;
	const char *reply_to_get = 0;
	const char *html_text =
		"<html><body>Hello from SentryPeer!</body></html>";
	char content_type_json[] = "application/json";
	bool json_requested = false;

	if (MHD_lookup_connection_value(connection, MHD_HEADER_KIND,
					MHD_HTTP_HEADER_CONTENT_TYPE) != NULL) {
		fprintf(stderr, "Content-Type is: %s\n",
			MHD_lookup_connection_value(
				connection, MHD_HEADER_KIND,
				MHD_HTTP_HEADER_CONTENT_TYPE));

		if (strncasecmp(MHD_lookup_connection_value(
					connection, MHD_HEADER_KIND,
					MHD_HTTP_HEADER_CONTENT_TYPE),
				content_type_json,
				strlen(content_type_json)) == 0) {
			json_t *api_reply_to_get_json =
				json_pack("{s:s, s:s, s:s}", "status", "OK",
					  "message", "Hello from SentryPeer!",
					  "version", PACKAGE_VERSION);
			reply_to_get = json_dumps(api_reply_to_get_json,
						  JSON_INDENT(2));
			json_requested = true;
			// Free the json object
			json_decref(api_reply_to_get_json);
		} else {
			reply_to_get = html_text;
		}
	} else {
		reply_to_get = html_text;
	}

	struct MHD_Response *response;
	// https://lists.gnu.org/archive/html/libmicrohttpd/2020-06/msg00013.html
	enum MHD_Result ret;

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
	response = MHD_create_response_from_buffer(strlen(reply_to_get),
						   (void *)reply_to_get,
						   MHD_RESPMEM_PERSISTENT);
	if (response != NULL) {
		if (json_requested &&
		    (MHD_add_response_header(response,
					     MHD_HTTP_HEADER_CONTENT_TYPE,
					     content_type_json) == MHD_NO)) {
			fprintf(stderr, "Failed to add header\n");
			MHD_destroy_response(response);
			return MHD_NO;
		}

		const struct sockaddr *addr =
			MHD_get_connection_info(
				connection, MHD_CONNECTION_INFO_CLIENT_ADDRESS)
				->client_addr;

		if (addr != NULL) {
			char client_ip_str[INET6_ADDRSTRLEN];
			inet_ntop(
				addr->sa_family,
				addr->sa_family == AF_INET ?
					(void *)&(((struct sockaddr_in *)addr)
							  ->sin_addr) :
					      (void *)&(((struct sockaddr_in6 *)addr)
							  ->sin6_addr),
				client_ip_str, sizeof(client_ip_str));

			fprintf(stderr, "GET %s from Client IP: %s\n", url,
				client_ip_str);
		}
		ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
		MHD_destroy_response(response);
		return ret;
	} else {
		return MHD_NO;
	}
}

int http_daemon_init(struct sentrypeer_config const *config)
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
