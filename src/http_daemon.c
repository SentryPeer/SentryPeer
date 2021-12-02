/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only */
/* Copyright (c) 2021 Gavin Henry <ghenry@sentrypeer.org> */

#include "conf.h"
#include "http_daemon.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>

#include <microhttpd.h>
#include <jansson.h>
#include <stdbool.h>

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

		if (strncmp(MHD_lookup_connection_value(
				    connection, MHD_HEADER_KIND,
				    MHD_HTTP_HEADER_CONTENT_TYPE),
			    content_type_json,
			    strlen(content_type_json)) == 0) {
			json_t *api_reply_to_get_json = json_pack(
				"{s:s, s:s}", "status", "OK", "message",
				"Hello from the SentryPeer RESTful API!");
			reply_to_get = json_dumps(api_reply_to_get_json,
						  JSON_INDENT(2));
			json_requested = true;
			// Free the json object
			//json_decref(api_reply_to_get_json);
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

	if (json_requested)
		MHD_add_response_header(response, MHD_HTTP_HEADER_CONTENT_TYPE,
					content_type_json);

	const struct sockaddr *addr =
		MHD_get_connection_info(connection,
					MHD_CONNECTION_INFO_CLIENT_ADDRESS)
			->client_addr;

	// IPv4 for now. Handle IPv6 later using https://www.mail-archive.com/libmicrohttpd@gnu.org/msg02421.html
	const char *client_ip =
		inet_ntoa(((struct sockaddr_in *)addr)->sin_addr);

	fprintf(stderr, "GET %s from %s\n", url, client_ip);

	ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
	MHD_destroy_response(response);
	return ret;
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
