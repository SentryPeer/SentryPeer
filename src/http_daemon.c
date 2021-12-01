/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only */
/* Copyright (c) 2021 Gavin Henry <ghenry@sentrypeer.org> */

#include "conf.h"
#include "http_daemon.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>

// https://lists.gnu.org/archive/html/libmicrohttpd/2021-12/msg00001.html
#include <microhttpd.h>

#define JSON "{\"title\":\"SentryPeer API\", \"body\":\"SentryPeer API Demo\"}"

static enum MHD_Result ahc_get(void *cls, struct MHD_Connection *connection,
			       const char *url, const char *method,
			       const char *version, const char *upload_data,
			       size_t *upload_data_size, void **ptr)
{
	static int dummy;
	const char *page = cls;
	struct MHD_Response *response;
	// https://lists.gnu.org/archive/html/libmicrohttpd/2020-06/msg00013.html
	enum MHD_Result ret;

	if (0 != strcmp(method, "GET"))
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
	response = MHD_create_response_from_buffer(strlen(page), (void *)page,
						   MHD_RESPMEM_PERSISTENT);

	MHD_add_response_header(response, "Content-Type", "application/json");

	const struct sockaddr *addr = MHD_get_connection_info(connection, MHD_CONNECTION_INFO_CLIENT_ADDRESS)->client_addr;

	// IPv4 for now. Handle IPv6 later using https://www.mail-archive.com/libmicrohttpd@gnu.org/msg02421.html
	const char *client_ip = inet_ntoa(((struct sockaddr_in *)addr)->sin_addr);

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
				  HTTP_DAEMON_PORT, NULL, NULL, &ahc_get, JSON,
				  MHD_OPTION_END);
	if (daemon == NULL) {
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;

	// MHD_stop_daemon(d);
}
